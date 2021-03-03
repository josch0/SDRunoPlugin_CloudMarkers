#include <iunoplugin.h>
#include "Plugin.h"

extern "C"
{

	UNOPLUGINAPI IUnoPlugin* UNOPLUGINCALL CreatePlugin(IUnoPluginController& controller)
	{
		return new Plugin(controller);
	}

	UNOPLUGINAPI void UNOPLUGINCALL DestroyPlugin(IUnoPlugin* plugin)
	{
		delete plugin;
	}

	UNOPLUGINAPI unsigned int UNOPLUGINCALL GetPluginApiLevel()
	{
		return UNOPLUGINAPIVERSION;
	}
}