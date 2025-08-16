%Module libqt_gui_cpp_sip

%Import QtCore/QtCoremod.sip
%Import QtGui/QtGuimod.sip
%Import QtWidgets/QtWidgetsmod.sip

%DefaultSupertype sip.simplewrapper

%Include composite_plugin_provider.sip
%Include generic_proxy.sip
%Include plugin.sip
%Include plugin_bridge.sip
%Include plugin_context.sip
%Include plugin_descriptor.sip
%Include plugin_provider.sip
%Include recursive_plugin_provider.sip
%Include ros_pluginlib_plugin_provider_for_plugin_providers.sip
%Include ros_pluginlib_plugin_provider_for_plugins.sip
%Include settings.sip
