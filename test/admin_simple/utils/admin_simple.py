from wizard.plugin import Plugin, PluginInfo

__plugin__ = PluginInfo('AdminSimple')

class AdminSimple(Plugin):
	def plugin_start(self):
		print('AdminSimple::plugin_start')

	def plugin_end(self):
		print('AdminSimple::plugin_end')
