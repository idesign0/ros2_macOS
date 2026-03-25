"""Helper functions for generating gz-msgs compatible messages"""

load("@rules_cc//cc:cc_library.bzl", "cc_library")
load("@rules_cc//cc/common:cc_common.bzl", "cc_common")
load("@rules_cc//cc/common:cc_info.bzl", "CcInfo")
load("@rules_proto//proto:defs.bzl", "ProtoInfo")

visibility("public")

_PROTO_EXTENSION = ".proto"
_VIRTUAL_IMPORTS = "/_virtual_imports/"

def _strip_proto_extension(proto_filename):
    if not proto_filename.endswith(_PROTO_EXTENSION):
        fail('"{}" does not end with "{}"'.format(
            proto_filename,
            _PROTO_EXTENSION,
        ))
    return proto_filename[:-len(_PROTO_EXTENSION)]

def proto_path_to_generated_filename(proto_path, fmt_str):
    """Calculates the name of a generated file for a protobuf path.

    For example, "examples/protos/helloworld.proto" might map to
      "helloworld.pb.h".
    Args:
      proto_path: The path to the .proto file.
      fmt_str: A format string used to calculate the generated filename. For
        example, "{}.pb.h" might be used to calculate a C++ header filename.
    Returns:
      The generated filename.
    """
    return fmt_str.format(_strip_proto_extension(proto_path))

def get_include_directory(source_file):
    """Returns the include directory path for the source_file.

    I.e. all of the include statements within the given source_file are
    calculated relative to the directory returned by this method.
    The returned directory path can be used as the "--proto_path=" argument
    value.
    Args:
      source_file: A proto file.
    Returns:
      The include directory path for the source_file.
    """
    directory = source_file.path
    prefix_len = 0

    if is_in_virtual_imports(source_file):
        root, relative = source_file.path.split(_VIRTUAL_IMPORTS, 2)
        result = root + _VIRTUAL_IMPORTS + relative.split("/", 1)[0]
        return result

    if not source_file.is_source and directory.startswith(source_file.root.path):
        prefix_len = len(source_file.root.path) + 1

    if directory.startswith("external", prefix_len):
        external_separator = directory.find("/", prefix_len)
        repository_separator = directory.find("/", external_separator + 1)
        return directory[:repository_separator]
    else:
        return source_file.root.path if source_file.root.path else "."

def _get_staged_proto_file(context, source_file):
    if source_file.dirname == context.label.package or \
       is_in_virtual_imports(source_file):
        # Current target and source_file are in same package
        return source_file
    else:
        # Current target and source_file are in different packages (most
        # probably even in different repositories)
        copied_proto = context.actions.declare_file(source_file.basename)
        context.actions.run_shell(
            inputs = [source_file],
            outputs = [copied_proto],
            command = "cp {} {}".format(source_file.path, copied_proto.path),
            mnemonic = "CopySourceProto",
        )
        return copied_proto

def protos_from_context(context):
    """Copies proto files to the appropriate location.

    Args:
      context: The ctx object for the rule.
    Returns:
      A list of the protos.
    """
    protos = []
    for src in context.attr.deps:
        for file in src[ProtoInfo].direct_sources:
            protos.append(_get_staged_proto_file(context, file))
    return protos

def declare_out_files(protos, context, generated_file_format):
    """Declares and returns the files to be generated.

    Args:
      protos: List of protos.
      context: The ctx object forwarded from the rule.
      generated_file_format: Formatting string for generated file name.
        e.g. "{}.pb.cc"
    Returns:
      A list of declared out files.
    """

    out_file_paths = []
    for proto in protos:
        if not is_in_virtual_imports(proto):
            out_file_paths.append(proto.basename)
        else:
            path = proto.path[proto.path.index(_VIRTUAL_IMPORTS) + 1:]
            out_file_paths.append(path)

    return [
        context.actions.declare_file(
            proto_path_to_generated_filename(
                out_file_path,
                generated_file_format,
            ),
        )
        for out_file_path in out_file_paths
    ]

def get_out_dir(protos, context):
    """ Returns the calculated value for --<lang>_out= protoc argument.

    Computed based on the input source proto files and current context.
    Args:
        protos: A list of protos to be used as source files in protoc command
        context: A ctx object for the rule.
    Returns:
        The value of --<lang>_out= argument.
    """
    at_least_one_virtual = 0
    for proto in protos:
        if is_in_virtual_imports(proto):
            at_least_one_virtual = True
        elif at_least_one_virtual:
            fail("Proto sources must be either all virtual imports or all real")
    if at_least_one_virtual:
        out_dir = get_include_directory(protos[0])
        return struct(
            path = out_dir,
            import_path = out_dir[out_dir.find(_VIRTUAL_IMPORTS) + 1:],
        )
    return struct(path = context.bin_dir.path, import_path = None)

def is_in_virtual_imports(source_file, virtual_folder = _VIRTUAL_IMPORTS):
    """Determines if source_file is virtual.

    I.e. whether the source_file is placed in the _virtual_imports subdirectory.
    The output of all proto_library targets which use import_prefix and/or
    strip_import_prefix arguments is placed under _virtual_imports directory.
    Args:
        source_file: A proto file.
        virtual_folder: The virtual folder name (is set to "_virtual_imports"
            by default)
    Returns:
        True if source_file is located under _virtual_imports, False otherwise.
    """
    return not source_file.is_source and virtual_folder in source_file.path

def _filter_files_impl(ctx):
    """Filter the files in DefaultInfo."""
    return [DefaultInfo(
        files = depset([
            file
            for file in ctx.attr.target.files.to_list()
            if file.extension in ctx.attr.extensions
        ]),
    )]

filter_files = rule(
    implementation = _filter_files_impl,
    attrs = {
        "target": attr.label(
            doc = "The source target to filter",
            mandatory = True,
        ),
        "extensions": attr.string_list(
            doc = "The extensions of the files to keep eg. ['h']",
            mandatory = True,
        ),
    },
)

def _gz_proto_factory_impl(ctx):
    """
    Implementation of gz_proto_factory rule
    """
    protos = protos_from_context(ctx)
    out_dir = get_out_dir(protos, ctx)
    in_protos = []
    include_dirs = depset([get_include_directory(proto) for proto in protos])

    for src in ctx.attr.deps:
        for proto in src[ProtoInfo].direct_sources:
            in_protos.append(proto)

    in_protos = depset(in_protos).to_list()
    arguments = [
        "--cc-output=" + ctx.outputs.cc_output.path,
        "--hh-output=" + ctx.outputs.hh_output.path,
    ]

    for include_dir in include_dirs.to_list():
        arguments.append("--proto-path=" + include_dir)

    arguments.append("--proto-include-path=" + out_dir.path)
    arguments.append("--protos")
    for proto in in_protos:
        arguments.append(proto.path)

    ctx.actions.run(
        inputs = protos,
        outputs = [ctx.outputs.cc_output, ctx.outputs.hh_output],
        arguments = arguments,
        executable = ctx.executable.gz_msgs_generate_factory_py,
    )

    compilation_context = cc_common.create_compilation_context(
        headers = depset([ctx.outputs.hh_output]),
        system_includes = depset([ctx.outputs.hh_output.dirname]),
    )

    return [
        CcInfo(compilation_context = compilation_context),
    ]

_gz_proto_factory = rule(
    attrs = {
        "deps": attr.label_list(
            mandatory = True,
            allow_empty = False,
            providers = [ProtoInfo],
        ),
        "cc_output": attr.output(mandatory = True),
        "hh_output": attr.output(mandatory = True),
        "gz_msgs_generate_factory_py": attr.label(
            default = Label("//tools:gz_msgs_generate_factory_py"),
            executable = True,
            cfg = "exec",
        ),
    },
    implementation = _gz_proto_factory_impl,
)

def gz_proto_factory(
        deps,
        **kwargs):
    """
    gz_proto_factory rule implementation wrapper
    """
    _gz_proto_factory(
        deps = deps,
        **kwargs
    )

def _gz_proto_library_impl(ctx):
    """
    Implementation of the gz_proto_library rule
    """
    protos = protos_from_context(ctx)
    out_dir = get_out_dir(protos, ctx)

    arguments = []

    in_protos = []
    out_protos = []
    include_dirs = []

    for src in ctx.attr.deps:
        for proto in src[ProtoInfo].direct_sources:
            in_protos.append(proto)
            include_dirs.append(get_include_directory(proto))
            out_protos.append(proto)

        for proto in src[ProtoInfo].transitive_sources.to_list():
            in_protos.append(proto)
            include_dirs.append(get_include_directory(proto))

    in_protos = depset(in_protos).to_list()
    out_protos = depset(out_protos).to_list()
    include_dirs = depset(include_dirs).to_list()

    for include_dir in include_dirs:
        arguments.append("--proto_path=" + include_dir)

    arguments.append("--plugin=protoc-gen-gzmsgs=" + ctx.executable._gz_gen_bin.path)
    arguments.append("--cpp_out=" + out_dir.path)
    arguments.append("--gzmsgs_out=" + out_dir.path)

    for proto in in_protos:
        arguments.append(proto.path)

    cc_files = declare_out_files(out_protos, ctx, "{}.pb.cc")
    hh_files = declare_out_files(out_protos, ctx, "{}.pb.h")
    out_files = cc_files + hh_files

    ctx.actions.run(
        inputs = in_protos,
        outputs = out_files,
        arguments = arguments,
        executable = ctx.executable._protoc,
        tools = [ctx.executable._protoc, ctx.executable._gz_gen_bin],
    )

    compilation_context = cc_common.create_compilation_context(
        headers = depset(hh_files),
        system_includes = depset([out_dir.path]),
    )

    return [
        DefaultInfo(files = depset(out_files)),
        CcInfo(compilation_context = compilation_context),
    ]

_gz_proto_library = rule(
    attrs = {
        "deps": attr.label_list(
            mandatory = True,
            allow_empty = False,
            providers = [ProtoInfo],
        ),
        "_protoc": attr.label(
            default = Label("@com_google_protobuf//:protoc"),
            executable = True,
            cfg = "exec",
        ),
        "_gz_gen_bin": attr.label(
            default = Label("//:gz_msgs_gen"),
            executable = True,
            cfg = "exec",
        ),
    },
    implementation = _gz_proto_library_impl,
)

def gz_proto_library(
        name,
        proto_deps,
        **kwargs):
    """gz_proto_library rule implementation wrapper

    Args:
        name: Rule name
        proto_deps: List of proto_library targets to be compiled and included in
          the wrapped cc_library rule.
        **kwargs: forwarded to wrapped cc_library.
    """
    _gz_proto_library(name = name + "_pb", deps = proto_deps)

    filter_files(name = name + "_srcs", target = ":" + name + "_pb", extensions = ["cc"])
    filter_files(name = name + "_hdrs", target = ":" + name + "_pb", extensions = ["h"])
    kwargs["deps"].append(":" + name + "_pb")
    cc_library(
        name = name,
        srcs = [name + "_srcs"],
        hdrs = [name + "_hdrs"],
        **kwargs
    )
