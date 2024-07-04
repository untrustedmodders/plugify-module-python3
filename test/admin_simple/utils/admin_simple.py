from plugify.plugin import Plugin


class AdminSimple(Plugin):
	def plugin_start(self):
		print('AdminSimple::plugin_start')

	def plugin_end(self):
		print('AdminSimple::plugin_end')
