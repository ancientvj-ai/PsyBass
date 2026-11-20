# PsyBass — sintetizador de bass para psytrance (VST3)

Synth monofônico feito em C++ com JUCE. Saw/Square + Sub, filtro com envelope,
drive, glide, largura estéreo e um **gate rítmico** (Offbeat 1/8 e Rolling 1/16)
que cria o "rolling bass" do psytrance a partir de UMA nota MIDI longa.

## Como usar no Ableton
1. Crie uma **trilha MIDI** e coloque o PsyBass nela.
2. Em PATTERN escolha **Rolling 1/16** e toque uma nota longa (ex.: F1 / F#1) durante 1 compasso ou mais.
3. O plugin corta o tempo do kick (1ª semicolcheia de cada tempo) e toca as outras 3 — o "rolling".
4. Mude a nota para mudar a linha de baixo. Use PRESET para ter pontos de partida.
5. Pattern **Off** = synth normal tocado só pelo MIDI.

## Opção A — compilar no GitHub (sem instalar nada)
1. Crie uma conta e um repositório novo em github.com (pode ser privado).
2. Envie TODOS os arquivos desta pasta (inclusive a pasta oculta `.github`).
3. Aba **Actions** -> workflow "Build PsyBass" -> espere ~10 min.
4. Baixe o artefato **PsyBass-VST3-Windows** (zip), extraia.
5. Copie a pasta **PsyBass.vst3** para `C:\Program Files\Common Files\VST3`.

## Opção B — compilar no seu PC
1. Instale **Visual Studio 2022 Community** (carga "Desenvolvimento para desktop com C++") e o **Git**.
2. Abra o "x64 Native Tools Command Prompt" na pasta do projeto e rode:
       cmake -B build
       cmake --build build --config Release --target PsyBass_VST3
3. O plugin fica em `build\PsyBass_artefacts\Release\VST3\PsyBass.vst3`.
   Copie para `C:\Program Files\Common Files\VST3`.
   (Também gera um .exe "Standalone" para testar sem a DAW.)

## No Ableton
Preferences -> Plug-ins -> ative "Use VST3 Plug-In System Folders" -> Rescan.

## Onde mexer no código
- `Source/PluginProcessor.cpp` -> som (osciladores, filtro, envelopes, presets)
- `Source/PluginEditor.cpp`    -> visual (cores, knobs, layout)
- Troque `COMPANY_NAME` e o nome do plugin no `CMakeLists.txt`.

## Licença do JUCE
JUCE tem licença gratuita com limites. Se for VENDER o plugin, confira os termos
atuais em juce.com antes.
