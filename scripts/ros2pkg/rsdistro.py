#!/usr/bin/env python3
"""
Export ROS 2 package metadata from rosdistro for multiple distros.

Self-contained: writes next to this script under ./manifests/, so the whole
system lives inside the repo and never depends on ~/Downloads.

Reads `source_repository` (real upstream url + branch + owner) — NOT
`release_repository` (the bloom `ros2-gbp/<repo>-release.git`, which you can
never submodule and whose owner is always ros2-gbp).

Outputs, per distro, into ./manifests/:
  <distro>_packages.json   full package-level metadata
  <distro>_repos.json      deduped repo-level view (the join/placement unit)
"""

import json
import os
import re
from urllib.parse import urlparse
from rosdistro import get_distribution, get_index, get_index_url

DISTROS = ["humble", "jazzy", "kilted"]
OUT_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "manifests")


def parse_owner_repo(url):
    if not url:
        return None, None
    u = re.sub(r"^git@([^:]+):", r"https://\1/", url.strip())
    path = re.sub(r"\.git$", "", urlparse(u).path.lstrip("/"))
    parts = path.split("/")
    if len(parts) < 2:
        return None, None
    return parts[0], parts[-1]


def norm_repo(name):
    if not name:
        return None
    n = re.sub(r"-release$", "", re.sub(r"^id_", "", re.sub(r"\.git$", "", name.lower())))
    return n


def export_distro(index, distro):
    dist = get_distribution(index, distro)
    packages_data, repos_data = {}, {}

    for pkg_name, pkg in sorted(dist.release_packages.items()):
        repo_name = pkg.repository_name
        repo_info = dist.repositories.get(repo_name) if repo_name else None
        rel = repo_info.release_repository if repo_info else None
        src = repo_info.source_repository if repo_info else None

        source_url = src.url if src else None
        source_branch = src.version if src else None
        owner, _ = parse_owner_repo(source_url)

        version = rel.version if rel else None
        tag = None
        if rel and rel.tags and "release" in rel.tags and version:
            tag = rel.tags["release"].replace("{package}", pkg_name).replace("{version}", version)

        packages_data[pkg_name] = {
            "package_name": pkg_name, "repository": repo_name,
            "source_url": source_url, "source_branch": source_branch, "owner": owner,
            "release_url": rel.url if rel else None, "version": version, "release_tag": tag,
        }

        if repo_name and repo_name not in repos_data:
            repos_data[repo_name] = {
                "repository": repo_name, "key": norm_repo(repo_name),
                "source_url": source_url, "source_branch": source_branch,
                "owner": owner, "packages": [],
            }
        if repo_name:
            repos_data[repo_name]["packages"].append(pkg_name)

    with open(os.path.join(OUT_DIR, f"{distro}_packages.json"), "w") as f:
        json.dump(packages_data, f, indent=2)
    with open(os.path.join(OUT_DIR, f"{distro}_repos.json"), "w") as f:
        json.dump(repos_data, f, indent=2)

    no_src = sum(1 for v in packages_data.values() if not v["source_url"])
    print(f"{distro}: {len(packages_data)} packages, {len(repos_data)} repos "
          f"({no_src} packages had no source_repository)")


def main():
    os.makedirs(OUT_DIR, exist_ok=True)
    index = get_index(get_index_url())
    available = set(index.distributions.keys())
    for distro in DISTROS:
        if distro not in available:
            print(f"skip {distro}: not in rosdistro index {sorted(available)}")
            continue
        export_distro(index, distro)


if __name__ == "__main__":
    main()
