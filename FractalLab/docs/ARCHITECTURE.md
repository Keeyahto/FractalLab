# ARCHITECTURE.md

# Fractal Lab — Architecture

## 1. Purpose of this document

Этот документ фиксирует базовую архитектуру проекта **Fractal Lab**: какие в нём есть подсистемы, за что они отвечают, как они взаимодействуют и какие технические решения считаются основными на старте.

Документ не пытается описать всё “академически правильно” и не является формальной спецификацией.  
Его цель — дать проекту **устойчивый каркас**, чтобы дальше можно было писать код без хаоса и без постоянного пересобирания концепции.

---

## 2. Architectural goal

Архитектура Fractal Lab должна поддерживать следующий сценарий:

1. Приложение рендерит в реальном времени 3D-фрактальную сцену.
2. Пользователь может свободно летать камерой.
3. Пользователь может менять параметры сцены на лету.
4. Аудио-сигнал анализируется в реальном времени.
5. Аудио-фичи модулируют визуальные параметры сцены.
6. Состояние сцены можно сохранять и загружать как пресеты.
7. В будущем можно добавить морфинг, новые фракталы, запись кадров, новые reactive-режимы и дополнительную визуальную логику без слома основы проекта.

---

## 3. Architectural principles

### 3.1. CPU controls, GPU renders
CPU отвечает за:
- состояние приложения
- UI
- камеру
- пресеты
- аудиоанализ
- reactive logic
- морфинг
- сборку финального состояния кадра

GPU отвечает за:
- вычисление изображения
- ray marching
- distance estimator evaluation
- normals
- shading
- fog / atmosphere
- запись кадра в render target

### 3.2. Parameter-driven architecture
Проект строится вокруг **системы параметров**, а не вокруг набора глобальных переменных.

Практически всё, что влияет на картинку, должно быть представлено как параметр:
- параметры фрактала
- параметры рендера
- параметры света
- параметры тумана
- параметры постэффектов
- параметры reactive-system
- параметры камеры, если это уместно

Это нужно для:
- UI
- пресетов
- аудиореактивности
- морфинга
- анимаций
- сериализации

### 3.3. Audio-reactive logic is a first-class system
Аудиореактивность не является “поздним прикручиванием FFT”.  
Это отдельная архитектурная подсистема, которая изначально встроена в pipeline кадра.

### 3.4. Presets are scene states
Пресет — это не просто список чисел.  
Пресет — это **сохранённое художественное состояние сцены**, включающее:
- активный тип фрактала
- значения параметров
- свет
- атмосферу
- палитру
- reactive bindings или reactive profile
- metadata

### 3.5. Extendable, but not overengineered
Архитектура должна быть расширяемой, но без premature abstraction.  
На старте не вводятся:
- плагины
- скриптовые DSL
- node graph
- distributed systems
- overly generic plugin pipelines

---

## 4. High-level system overview

Fractal Lab состоит из следующих ключевых подсистем:

- **App Layer**
- **Engine Core**
- **Scene System**
- **Parameter System**
- **Preset System**
- **Audio System**
- **Reactive System**
- **Morph System**
- **Renderer**
- **UI Layer**
- **Asset/Serialization Layer**

Ниже — краткое описание ролей.

### 4.1. App Layer
Отвечает за запуск приложения, создание окна, инициализацию подсистем и главный цикл.

### 4.2. Engine Core
Содержит глобальное runtime-состояние, время, orchestration логики и сборку `FrameState`.

### 4.3. Scene System
Описывает текущее художественное состояние сцены:
- камера
- активный фрактал
- свет
- туман
- постобработка
- временные значения

### 4.4. Parameter System
Содержит описание всех параметров проекта, их метаданные, значения и механизм их разрешения в итоговый state кадра.

### 4.5. Preset System
Сохраняет и загружает сцены, переключает художественные состояния и даёт базу для морфинга.

### 4.6. Audio System
Получает аудио, буферизует его, извлекает спектральные и временные признаки.

### 4.7. Reactive System
Преобразует audio features в модуляции параметров.

### 4.8. Morph System
Отвечает за переходы между двумя состояниями сцены или пресетами.

### 4.9. Renderer
Вычисляет кадр на GPU и выводит его на экран через OpenGL interop.

### 4.10. UI Layer
Даёт пользователю контроль над сценой, reactive bindings, пресетами и debug-информацией.

---

## 5. Main architectural idea

Центральная идея Fractal Lab:

> итоговый вид кадра определяется не одним источником данных, а композицией нескольких слоёв параметров.

Итоговый параметр сцены формируется из:

- базового пресета
- ручных пользовательских изменений
- морфинга
- автоматических анимаций
- аудиореактивных модуляций
- runtime clamping / safety rules

Формально:

```text
Resolved Parameters
  = Base Preset
  + UI Overrides
  + Morph Layer
  + Animation Layer
  + Audio Reactive Layer
  -> Clamp / Sanitize
````

Это означает, что никакая подсистема не должна напрямую “переписывать всё состояние сцены”.
Она должна вносить свой вклад в систему параметров.

---

## 6. Data flow per frame

Каждый кадр должен проходить через следующую цепочку:

```text
Input / Window Events
    ↓
Camera Update
    ↓
Audio Capture
    ↓
FFT / Feature Extraction
    ↓
Reactive Evaluation
    ↓
Morph / Animation Update
    ↓
Parameter Resolution
    ↓
FrameState Build
    ↓
GPU Render
    ↓
Display
    ↓
UI Draw
```

Это основной pipeline приложения.

---

## 7. Core runtime objects

## 7.1. EngineState

`EngineState` — главный runtime-контейнер приложения.
Он содержит текущее состояние всех подсистем.

Примерно логически:

```text
EngineState
├─ AppState
├─ TimeState
├─ SceneState
├─ CameraState
├─ ParameterState
├─ PresetState
├─ AudioState
├─ ReactiveState
├─ MorphState
├─ RendererState
├─ UIState
└─ DebugState
```

### Назначение

* является главным state-контейнером на CPU
* обновляется каждый кадр
* используется для сборки `FrameState`

---

## 7.2. FrameState

`FrameState` — компактное итоговое состояние кадра, которое передаётся в рендерер.

В отличие от `EngineState`, это уже не “вся логика приложения”, а только то, что реально нужно GPU для рендера текущего кадра.

Логически:

```text
FrameState
├─ CameraData
├─ ActiveFractalType
├─ FractalParams
├─ LightingParams
├─ FogParams
├─ PostFXParams
├─ RenderSettings
├─ TimeData
└─ Optional Reactive Summary
```

### Назначение

* собираться каждый кадр после resolution всех параметров
* быть компактным и предсказуемым
* быть стабильным контрактом между CPU и GPU

---

## 8. Module breakdown

# 8.1. App Layer

### Responsibility

* startup / shutdown
* создание окна
* init OpenGL / CUDA / ImGui
* запуск главного цикла
* координация загрузки initial preset

### Main entities

* `App`
* `Window`
* `InputRouter`

### Notes

Этот слой должен быть тонким.
Бизнес-логика сцены не должна жить здесь.

---

# 8.2. Engine Core

### Responsibility

* держать `EngineState`
* обновлять время
* вызывать обновление подсистем в правильном порядке
* собирать `FrameState`
* запускать рендер кадра

### Main entities

* `Engine`
* `UpdateLoop`
* `FrameBuilder`
* `TimeState`

### Notes

Это orchestration layer, а не место для сложной предметной логики фракталов.

---

# 8.3. Scene System

### Responsibility

* описывать текущее состояние художественной сцены
* хранить семантически значимые части кадра

### Main entities

* `SceneState`
* `Camera`
* `LightingState`
* `FogState`
* `PostFXState`

### SceneState includes

* active fractal selection
* scene-local parameters
* current palette
* light settings
* fog settings
* environment mood values
* optional camera bookmarks / spawn info

### Notes

Scene System должен быть максимально “читаемым” по смыслу.
Это слой художественного состояния, а не внутренней рендер-математики.

---

# 8.4. Fractal System

### Responsibility

* описывать поддерживаемые фрактальные семейства
* связывать активный тип фрактала с нужными параметрами
* формировать GPU-side selection mode
* определять доступные ranges и metadata для UI

### Main entities

* `FractalType`
* `FractalRegistry`
* `FractalDescriptor`
* `FractalParamSchema`

### Examples of fractal types

* Mandelbulb
* Mandelbox
* Julia3D / Quaternion-style variants
* Hybrid / Fold-based fractals

### Notes

На старте реальная реализация может содержать только один фрактал, но registry и type system должны существовать сразу.

---

# 8.5. Parameter System

## Role

Это центральная архитектурная подсистема проекта.

Она нужна, чтобы:

* все управляемые характеристики сцены были единообразны
* UI мог автоматически их показывать
* пресеты могли их сериализовать
* audio-reactive system могла их модулировать
* morph system могла их интерполировать

## Main entities

* `ParameterId`
* `ParameterType`
* `ParameterMetadata`
* `ParameterValue`
* `ParameterRegistry`
* `ParameterSet`
* `ParameterResolver`

### ParameterMetadata should include

* unique id
* display name
* category
* type
* min/max
* default
* reactive-enabled flag
* interpolation mode
* serialization flag
* UI hints

### Parameter categories

* `fractal`
* `render`
* `lighting`
* `fog`
* `postfx`
* `camera`
* `audio`
* `reactive`
* `debug`

### Parameter types

* float
* int
* bool
* enum
* vec2 / vec3 / vec4
* color

---

## 8.5.1. Parameter storage model

Система параметров должна поддерживать разные слои данных.

Логически:

```text
Parameter Layers
├─ Default Layer
├─ Preset Layer
├─ UI Override Layer
├─ Morph Layer
├─ Animation Layer
├─ Audio Reactive Layer
└─ Safety / Clamp Layer
```

Итоговое разрешение параметра делается через `ParameterResolver`.

---

## 8.5.2. Resolution rules

Для каждого параметра нужно определить, как он собирается:

* additive
* multiplicative
* replace
* max/min-based
* trigger-based for event-like values

Примеры:

* `bloom_intensity` может быть additive
* `global_scale` может быть multiplicative
* `fractal_type` должен быть replace/discrete
* `drop_event` должен быть trigger/event signal

### Important rule

Нельзя позволять подсистемам напрямую переписывать “главное значение параметра” без участия `ParameterResolver`.

---

# 8.6. Preset System

## Role

Preset System позволяет сохранить и восстановить художественное состояние сцены.

## Main entities

* `Preset`
* `PresetManager`
* `PresetLoader`
* `PresetSerializer`
* `PresetInterpolator`

## Preset should contain

* preset id / name
* active fractal type
* parameter values
* lighting values
* fog / atmosphere values
* palette values
* reactive profile or bindings
* metadata:

  * author
  * description
  * tags
  * mood label

## Notes

Пресет — это главный пользовательский формат работы со сценами.

---

## 8.6.1. Why presets matter architecturally

Пресеты нужны не только для удобства пользователя, но и как база для:

* морфинга
* quick scene switching
* artistic workflow
* сохранения удачных состояний
* reproducibility

---

# 8.7. Audio System

## Role

Audio System получает аудио, буферизует его и преобразует в набор устойчивых музыкальных признаков.

## Main entities

* `AudioInput`
* `AudioBuffer`
* `FFTAnalyzer`
* `FeatureExtractor`
* `AudioFeatures`
* `AudioSmoother`

## Input sources

На старте желательно поддерживать:

* microphone
* loopback / system audio, если доступно на платформе
* audio file playback input, позже

## Processing stages

1. audio capture
2. buffer write
3. windowing
4. FFT
5. energy band extraction
6. temporal analysis
7. smoothing
8. feature publication

---

## 8.7.1. AudioFeatures

Система должна работать не с сырыми бин-значениями FFT, а с музыкально-полезными фичами.

Примеры полезных признаков:

* `rms`
* `low_energy`
* `mid_energy`
* `high_energy`
* `sub_bass_energy`
* `spectral_flux`
* `onset_strength`
* `transient_strength`
* `beat_confidence`
* `drop_intensity`
* `silence_factor`

### Why this matters

Reactive logic, завязанная на сырые FFT bins, быстро становится хаотичной и неудобной.
Reactive logic, завязанная на осмысленные признаки, даёт контролируемый художественный результат.

---

## 8.7.2. Smoothing and temporal behavior

Аудио без сглаживания будет слишком дёрганым.
Поэтому Audio System должна поддерживать:

* attack
* release
* damping
* peak hold
* normalization
* gain compensation

Это важно для ощущения “музыкальной живости” вместо “случайной тряски”.

---

# 8.8. Reactive System

## Role

Reactive System переводит audio features в визуальные модуляции параметров.

Это отдельный слой между `AudioSystem` и `ParameterResolver`.

## Main entities

* `ReactiveBinding`
* `ReactiveProfile`
* `ReactiveEngine`
* `ReactiveCurve`
* `ReactiveRouter`

---

## 8.8.1. ReactiveBinding

Каждая reactive-связка описывает правило вида:

```text
Audio Source → Parameter Target
```

Полезные поля:

* `source_feature`
* `target_parameter`
* `mode`
* `amount`
* `curve`
* `attack_ms`
* `release_ms`
* `bias`
* `scale`
* `clamp_min`
* `clamp_max`
* `enabled`

### Modes

* add
* multiply
* replace
* trigger
* gate

### Example

```text
source_feature = low_energy
target_parameter = fractal.scale
mode = add
amount = 0.12
curve = smoothstep
attack_ms = 25
release_ms = 180
```

---

## 8.8.2. ReactiveProfile

`ReactiveProfile` — это набор reactive bindings, объединённых в художественно осмысленный режим.

Например:

* `Soft Pulse`
* `Bass Cathedral`
* `Neon Twitch`
* `Drop Bloom`
* `Aggressive Ritual`

### Why profiles matter

Они позволяют быстро переключать характер поведения сцены без ручного конфигурирования десятков биндов.

---

## 8.8.3. Reactive evaluation

Reactive Engine каждый кадр:

1. получает `AudioFeatures`
2. применяет smoothing / curves / gain
3. вычисляет значения modulation outputs
4. пишет результат в `Audio Reactive Layer` системы параметров

---

# 8.9. Morph System

## Role

Morph System отвечает за плавный переход между двумя состояниями.

На архитектурном уровне морфинг — это не отдельный способ “магически менять сцену”, а ещё один слой модификации параметров.

## Main entities

* `MorphState`
* `MorphEngine`
* `PresetInterpolator`
* `TransitionCurve`

## Supported use cases

* preset A → preset B
* scene state A → scene state B
* fractal parameter state A → state B
* future: cross-scene transitions

---

## 8.9.1. Interpolation strategy

Нужно поддержать разные стратегии интерполяции по типу параметра:

* float → linear / smoothstep / cubic
* int → rounded interpolation
* color → linear color blend
* bool → threshold switch
* enum → discrete switch
* fractal type → special transition logic

### Important note

Морфинг между принципиально разными фрактальными семействами может не давать “математически красивый” переход.
Поэтому поддержка cross-fractal morphing должна проектироваться как художественная функция, а не как гарантированно универсальная операция.

---

# 8.10. Renderer

## Role

Renderer вычисляет изображение на GPU и отображает его в окне.

## Main responsibilities

* управление render target
* CUDA kernel launch
* CUDA ↔ OpenGL interop
* framebuffer output
* resize handling
* performance stats

## Main entities

* `Renderer`
* `CudaInteropSurface`
* `RenderTarget`
* `RenderKernel`
* `RenderSettings`

---

## 8.10.1. Renderer pipeline

Базовый pipeline кадра:

1. CPU собирает `FrameState`
2. `FrameState` отправляется на GPU
3. CUDA kernel рендерит изображение
4. результат записывается в GPU texture / surface
5. OpenGL показывает fullscreen quad
6. поверх рисуется ImGui

---

## 8.10.2. GPU responsibilities

GPU должен заниматься только визуальными вычислениями:

* ray generation
* marching
* DE evaluation
* hit detection
* normal estimation
* shading
* fog / atmosphere
* optional glow preparation

GPU не должен знать про:

* пресеты как художественные сущности
* UI
* аудио-систему как систему
* морфинг как продуктовую фичу

Он получает уже разрешённый `FrameState`.

---

## 8.10.3. Fractal rendering approach

Основной подход:

* **ray marching**
* **distance estimator fractals**

### Why

Это естественно подходит для:

* сложных процедурных поверхностей
* интерактивных зумов и полётов
* гибкой смены формулы
* богатого освещения без mesh generation

---

## 8.10.4. Render settings

Нужен отдельный блок render settings:

* max steps
* hit epsilon
* max distance
* normal epsilon
* shadow steps
* AO steps
* resolution scale
* quality preset

Эти значения должны быть частью параметризуемого pipeline, а не жёстко зашиты.

---

# 8.11. UI Layer

## Role

UI обеспечивает управление проектом и отладку.

## Main panels

* Main scene panel
* Fractal parameters panel
* Render settings panel
* Audio panel
* Reactive bindings panel
* Preset panel
* Debug / profiler panel

## Main principles

* UI не должен содержать предметную логику
* UI читает и пишет данные через Parameter System / managers
* UI — это control surface, а не business logic host

---

# 8.12. Asset / Serialization Layer

## Role

Слой сериализации отвечает за:

* загрузку пресетов
* сохранение пресетов
* загрузку палитр
* хранение пользовательских конфигов
* future: camera bookmarks / sessions

## Format

Для старта подойдут:

* JSON
* TOML

### Recommendation

Если хочется максимальной простоты — JSON.
Если хочется чуть более “человеческий” конфиг-стиль — TOML.

---

## 9. Main runtime loop

Ниже — базовый цикл приложения.

```text
while (app.isRunning())
{
    1. poll window/input events
    2. update time
    3. update camera controls
    4. capture/process audio
    5. extract audio features
    6. evaluate reactive bindings
    7. update morph/automation systems
    8. resolve final parameters
    9. build FrameState
    10. upload FrameState to GPU
    11. dispatch CUDA render
    12. present image via OpenGL
    13. draw ImGui
}
```

---

## 10. Threading model

На старте разумно держать модель простой.

### Initial model

* main thread:

  * window
  * input
  * UI
  * orchestration
  * rendering dispatch
* audio thread / callback:

  * capture audio samples into thread-safe ring buffer

### Optional later

Позже можно вынести:

* more advanced audio analysis
* async preset loading
* offline export

Но на старте главное — простота и предсказуемость.

---

## 11. Folder structure

Предлагаемая структура проекта:

```text
FractalLab/
├─ CMakeLists.txt
├─ app/
│  ├─ main.cpp
│  ├─ App.h
│  └─ App.cpp
├─ engine/
│  ├─ Engine.h
│  ├─ Engine.cpp
│  ├─ EngineState.h
│  ├─ FrameState.h
│  ├─ FrameBuilder.h
│  ├─ FrameBuilder.cpp
│  └─ TimeState.h
├─ scene/
│  ├─ SceneState.h
│  ├─ Camera.h
│  ├─ Camera.cpp
│  ├─ LightingState.h
│  ├─ FogState.h
│  └─ PostFXState.h
├─ fractal/
│  ├─ FractalType.h
│  ├─ FractalRegistry.h
│  ├─ FractalRegistry.cpp
│  ├─ FractalDescriptor.h
│  └─ schemas/
├─ params/
│  ├─ ParameterId.h
│  ├─ ParameterType.h
│  ├─ ParameterValue.h
│  ├─ ParameterMetadata.h
│  ├─ ParameterRegistry.h
│  ├─ ParameterRegistry.cpp
│  ├─ ParameterSet.h
│  ├─ ParameterSet.cpp
│  ├─ ParameterResolver.h
│  └─ ParameterResolver.cpp
├─ preset/
│  ├─ Preset.h
│  ├─ PresetManager.h
│  ├─ PresetManager.cpp
│  ├─ PresetLoader.h
│  ├─ PresetLoader.cpp
│  ├─ PresetSerializer.h
│  └─ PresetInterpolator.cpp
├─ audio/
│  ├─ AudioInput.h
│  ├─ AudioInput.cpp
│  ├─ AudioBuffer.h
│  ├─ FFTAnalyzer.h
│  ├─ FFTAnalyzer.cpp
│  ├─ AudioFeatures.h
│  ├─ FeatureExtractor.h
│  ├─ FeatureExtractor.cpp
│  ├─ AudioSmoother.h
│  └─ AudioSmoother.cpp
├─ reactive/
│  ├─ ReactiveBinding.h
│  ├─ ReactiveProfile.h
│  ├─ ReactiveEngine.h
│  ├─ ReactiveEngine.cpp
│  ├─ ReactiveCurve.h
│  └─ ReactiveRouter.cpp
├─ morph/
│  ├─ MorphState.h
│  ├─ MorphEngine.h
│  ├─ MorphEngine.cpp
│  └─ TransitionCurve.h
├─ render/
│  ├─ Renderer.h
│  ├─ Renderer.cpp
│  ├─ RenderSettings.h
│  ├─ gl/
│  │  ├─ DisplayQuad.h
│  │  ├─ DisplayQuad.cpp
│  │  ├─ GLTextureSurface.h
│  │  └─ CudaGlInterop.cpp
│  └─ cuda/
│     ├─ FractalRenderer.cu
│     ├─ DistanceEstimators.cuh
│     ├─ Shading.cuh
│     ├─ Normals.cuh
│     ├─ Fog.cuh
│     └─ RenderCommon.cuh
├─ ui/
│  ├─ MainPanel.cpp
│  ├─ FractalPanel.cpp
│  ├─ AudioPanel.cpp
│  ├─ ReactivePanel.cpp
│  ├─ PresetPanel.cpp
│  ├─ RenderPanel.cpp
│  └─ DebugPanel.cpp
├─ assets/
│  ├─ presets/
│  ├─ palettes/
│  └─ config/
└─ docs/
   ├─ VISION.md
   ├─ ARCHITECTURE.md
   └─ ROADMAP.md
```

---

## 12. Contracts between systems

Чтобы проект не расползался, нужно с самого начала зафиксировать несколько жёстких контрактов.

### Contract 1: CPU vs GPU

GPU получает только `FrameState` и render resources.
GPU не работает напрямую с UI, preset objects, audio managers и прочими high-level сущностями.

### Contract 2: Parameters are the integration surface

Если подсистема хочет влиять на сцену, она должна делать это через Parameter System или через контролируемую сборку `FrameState`.

### Contract 3: Audio does not mutate scene directly

Audio System не должен “сам менять фрактал”.
Он публикует `AudioFeatures`.
Reactive System решает, как эти features превращаются в модуляции.

### Contract 4: UI does not own logic

UI не хранит и не определяет поведение систем.
Он только отображает и редактирует данные.

### Contract 5: Presets are portable scene states

Preset должен быть самодостаточным форматом для воспроизведения художественного состояния.

---

## 13. Suggested technology stack

### Core

* C++17 or C++20
* CUDA
* OpenGL
* GLFW
* Dear ImGui
* GLM

### Serialization

* nlohmann/json
  или
* toml++

### Audio

* miniaudio как pragmatic start
* FFTW или другая лёгкая FFT-библиотека

### Build

* CMake

---

## 14. Future extension points

Архитектура должна позволять добавить позже:

### Rendering

* accumulation
* AO
* soft shadows
* better atmospherics
* multi-pass post-processing
* supersampling modes

### Audio

* beat detection improvements
* drop heuristics
* adaptive normalization
* profile-specific analysis

### Scene control

* timeline automation
* LFO generators
* scripted transitions
* camera paths

### Output

* screenshots
* frame sequence export
* offline high-quality render mode

---

## 15. Risks and architectural safeguards

### Risk 1: Global variable chaos

Если параметры разъедутся по коду как случайные поля и глобалки, reactive logic и presets быстро станут болью.

**Safeguard:** всё важное проходит через Parameter System.

### Risk 2: Audio-reactive spaghetti

Если маппинг аудио будет размазан по UI, renderer и scene code, систему станет невозможно контролировать.

**Safeguard:** отдельный Reactive Engine.

### Risk 3: GPU becomes product-logic host

Если логика пресетов, морфинга и high-level behavior полезет в CUDA kernel, поддержка станет тяжёлой.

**Safeguard:** GPU only renders resolved frame state.

### Risk 4: Premature engine-building

Есть риск потратить месяцы на “идеальную платформу”, не получив живого результата.

**Safeguard:** архитектура должна быть чистой, но первый практический приоритет — красивый рабочий кадр и живая reactive-сцена.

---

## 16. Minimal architecture that must exist from day one

Даже если MVP очень маленький, на старте должны существовать следующие вещи:

* `EngineState`
* `FrameState`
* `SceneState`
* `FractalType`
* `ParameterRegistry`
* `ParameterSet`
* `ParameterResolver`
* `AudioFeatures`
* `ReactiveBinding`
* `Renderer`

Не обязательно сразу полностью “богатые”, но они должны существовать как каркас.

---

## 17. Architecture summary

Fractal Lab строится как **parameter-driven realtime fractal engine** с отдельными подсистемами для:

* сцены
* пресетов
* аудиоанализа
* reactive logic
* морфинга
* GPU-рендера

Главный принцип проекта:

> все художественные и поведенческие изменения должны сходиться в систему параметров, а итоговый `FrameState` должен быть единственным контрактом между логикой приложения и GPU-рендером.

Это даёт:

* чистую расширяемость
* понятный data flow
* поддержку аудиореактивности как core-фичи
* основу для морфинга и пресетов
* технически устойчивую базу для дальнейшего роста проекта

---

## 18. One-line architectural statement

> Fractal Lab is a realtime CUDA-based fractal rendering application where scene behavior is driven by layered parameters resolved from presets, user control, morphing, and audio-reactive modulation.
