# Notes sur JUCE

Prendre le plus de temps de dev sur `prepareToPlay()` et `processBlock`

Le projet ne voit pas tous les symboles des modules JUCE. Pour configurer intellisense, dans le JSON C++, ajouter:
```
"includePath": [
                "${workspaceFolder}/**",
                "C:/Users/jcbsk/Documents/JUCE/modules/"
            ],
```



## 4 projets sont fournis dans un repo JUCE

### shared_code

This is the main library of your plugin's code. It includes the audio processing logic, parameter management, and any shared resources or utility functions.

Other projects like StandalonePlugin and VST3 depend on Shared_Code because it contains the core functionality of your plugin.


### VST3ManifestHelper

Purpose:
    This small project generates additional metadata or manifests required for the VST3 format.
    It is used during the VST3 build process to comply with the VST3 standard.
When to Build:
    You don't need to build this manually; it is typically triggered automatically when building the VST3 project.


For quick testing without a DAW:

    Build and run the StandalonePlugin project. This is ideal for development since it allows rapid testing of your plugin's functionality in isolation.

For testing inside a DAW:

    Build the VST3 project. After building, load the resulting .vst3 file into your DAW.


## Audiopluginhost

`.jucer` file -> Il faut l'ouvrir avec projucer.exe

1. Compile the project (open in VS, build) -> This will create the builds folder with .exe in it.
2. Open the exe, path is `\Builds\VisualStudio2022\x64\Debug\App\AudioPluginHost.exe`
3. Options -> edit list of available plugins -> add path to project(s), or VST3 folder of your project