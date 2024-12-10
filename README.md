# Notes sur JUCE

Prendre le plus de temps de dev sur `prepareToPlay()` et `processBlock`

Le projet ne voit pas tous les symboles des modules JUCE. Pour configurer intellisense, dans le JSON C++, ajouter:
```
"includePath": [
                "${workspaceFolder}/**",
                "C:/Users/jcbsk/Documents/JUCE/modules/"
            ],
```