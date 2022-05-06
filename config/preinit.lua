-- PreInit script - done first thing once main is executed

ConfigCategory("vulkan")
ConfigAddString("device.preferred", "NVIDIA")
ConfigAddbool("devices.loginfo", true)
ConfigAddBool("debug", true)
ConfigAddBool("require.queue.graphics", true)
ConfigAddBool("require.queue.compute", false)
ConfigAddBool("require.queue.transfer", false)

ConfigCategory("log")
ConfigAddBool("verbose", true)
ConfigAddBool("info", true)
ConfigAddBool("warning", true)
ConfigAddBool("error", true)
