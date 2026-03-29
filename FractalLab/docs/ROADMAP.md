# ROADMAP.md

# Fractal Lab — Roadmap

## 1. Purpose of this document

Этот документ фиксирует поэтапный план разработки **Fractal Lab**.

Его задача — не “предсказать весь проект наперёд”, а:

- удерживать фокус
- не расползаться по фичам слишком рано
- понимать, что считается законченным результатом на каждом этапе
- отделять **ядро проекта** от **приятных расширений**

Roadmap специально построен так, чтобы проект как можно раньше начал давать **живой визуальный результат**, а не превращался в бесконечную подготовку инфраструктуры.

---

## 2. Main development strategy

Fractal Lab должен развиваться не как “сразу строим всё”, а как последовательность слоёв:

1. **Сначала** — получить первый красивый realtime-фрактальный кадр.
2. **Потом** — сделать сцену управляемой и приятной для исследования.
3. **Потом** — встроить настоящую аудиореактивность как core-feature.
4. **Потом** — расширять художественные возможности: пресеты, новые фракталы, морфинг, атмосфера, экспорт.

Главный принцип roadmap:

> каждый этап должен заканчиваться состоянием проекта, которым уже приятно пользоваться.

---

## 3. Product priorities

### 3.1. Absolute priorities
Это то, ради чего проект вообще делается.

- realtime 3D fractal rendering
- free camera exploration
- live parameter control
- audio-reactive behavior
- strong “zaliпательный” visual feel

### 3.2. High priorities
Это очень важно, но не должно ломать скорость ранней разработки.

- preset system
- expressive shading
- fog / atmosphere
- multiple fractal families
- stable reactive mappings

### 3.3. Secondary priorities
Это уже сильные усилители проекта, но не ядро первого рабочего результата.

- morphing between presets
- progressive / accumulation rendering
- recording / export
- advanced postfx
- timeline-like automation

---

## 4. Milestone overview

Roadmap делится на следующие этапы:

- **Phase 0 — Project Skeleton**
- **Phase 1 — First Living Frame**
- **Phase 2 — Usable Explorer**
- **Phase 3 — Audio-Reactive Core**
- **Phase 4 — Presets and Performance Feel**
- **Phase 5 — Multi-Fractal Expansion**
- **Phase 6 — Morphing and Transitions**
- **Phase 7 — Polish and Output**
- **Phase X — Future Extensions**

---

# Phase 0 — Project Skeleton

## Goal
Поднять чистый каркас проекта, чтобы дальше не писать всё в один файл и не переделывать стартовую основу.

## What should exist
- базовая структура директорий
- CMake-проект
- окно через GLFW
- OpenGL context
- базовая интеграция Dear ImGui
- подключённый CUDA toolchain
- пустой render loop
- заготовки для `EngineState`, `FrameState`, `Renderer`

## Deliverable
Приложение запускается, открывает окно, работает render loop, рисует пустой экран или тестовый цвет, поверх есть ImGui.

## Exit criteria
Фаза завершена, если:
- проект собирается и запускается стабильно
- окно resize’ится без проблем
- есть минимальный runtime loop
- есть место, куда дальше можно вставлять рендер-логику

## Important note
На этом этапе нельзя застрять надолго.  
Это только стартовая инфраструктура, не самостоятельная цель.

---

# Phase 1 — First Living Frame

## Goal
Получить **первый настоящий интерактивный кадр** с 3D-фракталом.

Это самый важный ранний этап всего проекта.

## Scope
- CUDA renderer
- output в GPU texture / buffer
- OpenGL display
- один базовый фрактал: **Mandelbulb**
- ray marching
- basic hit detection
- estimation of normals
- простое shading:
  - diffuse
  - specular
- свободная камера:
  - WASD
  - мышь
  - speed control

## Deliverable
Пользователь запускает приложение и может летать вокруг или внутри Mandelbulb в realtime.

## Exit criteria
Фаза завершена, если:
- фрактал реально рендерится на GPU
- есть живая камера
- управление ощущается адекватно
- при изменении позиции камеры картинка не разваливается катастрофически
- сцена уже визуально интересна, даже без сложной атмосферы

## Non-goals
Пока не нужны:
- аудио
- пресеты
- морфинг
- несколько фракталов
- сложный postfx
- perfect optimization

## Why this phase matters
Если эта фаза не доведена до приятного состояния, весь остальной проект будет строиться на шаткой базе.

---

# Phase 2 — Usable Explorer

## Goal
Превратить “первый кадр” в **реально приятный интерактивный explorer**, а не просто техдемо.

## Scope
- `ParameterRegistry`
- `ParameterSet`
- `ParameterResolver` в минимальном рабочем виде
- UI для основных параметров
- live control:
  - fractal power
  - iterations
  - epsilon
  - max steps
  - lighting intensity
  - fog density
  - palette controls
- базовые atmosphere features:
  - fog
  - background color / gradient
- quality settings:
  - resolution scale
  - render quality preset

## Deliverable
Пользователь может не только летать, но и вживую менять характер сцены через UI.

## Exit criteria
Фаза завершена, если:
- важные параметры регулируются на лету
- архитектурно они уже проходят через Parameter System
- можно быстро получать разные визуальные состояния без перекомпиляции
- проект начинает ощущаться как “инструмент”

## Non-goals
Пока не нужны:
- полноценный preset manager
- audio-reactive system
- advanced AO / shadows
- morph system

## Important note
Именно здесь надо убедиться, что параметрическая архитектура удобна.  
Если сейчас всё неудобно, дальше с reactive и presets будет больно.

---

# Phase 3 — Audio-Reactive Core

## Goal
Встроить **настоящую аудиореактивность** как одну из центральных систем проекта.

Это один из самых важных этапов с точки зрения идентичности Fractal Lab.

## Scope

### Audio input
- захват аудио
- ring buffer
- поддержка хотя бы одного реального источника:
  - microphone  
  или
  - system audio loopback

### Analysis
- FFT
- energy bands:
  - low
  - mid
  - high
- RMS
- onset / transient estimation
- smoothing:
  - attack
  - release

### Reactive system
- `AudioFeatures`
- `ReactiveBinding`
- `ReactiveEngine`
- binding UI
- simple reactive modes:
  - bass → scale / pulse
  - mids → form detail / fold-like param
  - highs → palette / bloom-like intensity
  - onset → flash / spike

### UX
- enable / disable reactive mode
- per-binding intensity
- smoothing controls
- master sensitivity

## Deliverable
Пользователь включает музыку, и сцена начинает **понятно и красиво** реагировать на звук.

## Exit criteria
Фаза завершена, если:
- реакция на аудио есть в realtime
- реакция ощущается музыкальной, а не случайной
- существует хотя бы несколько управляемых reactive bindings
- пользователь может настраивать интенсивность реакций
- project feel становится реально “залипательным”

## Non-goals
Пока не нужны:
- идеальный beat detection
- сложный drop classifier
- суперточный music intelligence
- cross-preset transitions by events

## Important note
Главный критерий успеха здесь — **не техническая сложность анализа**, а художественная убедительность реакции.

---

# Phase 4 — Presets and Performance Feel

## Goal
Сделать так, чтобы Fractal Lab можно было использовать как **набор художественных состояний**, между которыми удобно переключаться.

## Scope
- `Preset`
- `PresetManager`
- JSON/TOML serialization
- save/load presets
- preset browser in UI
- palette presets
- lighting / fog / reactive profile inclusion
- несколько вручную собранных сцен, например:
  - Acid Temple
  - Organic Tunnel
  - Neon Coral
  - Void Cathedral

## Deliverable
Пользователь может загружать готовые “вайбы” и сохранять свои удачные сцены.

## Exit criteria
Фаза завершена, если:
- можно сохранить текущее состояние сцены
- можно восстановить его позже
- reactive bindings тоже входят в художественное состояние
- проект начинает ощущаться как коллекция playable worlds

## Non-goals
Пока не нужны:
- сложная библиотека пресетов
- cloud sync
- community sharing
- morphing between presets

## Why this phase matters
Без пресетов проект будет каждый раз начинаться “с нуля”.  
С пресетами он получает память и художественную повторяемость.

---

# Phase 5 — Multi-Fractal Expansion

## Goal
Перейти от “одного хорошего фрактала” к **малому фрактальному набору**, который делает проект богаче.

## Scope
- `FractalRegistry`
- как минимум ещё 1–2 семейства фракталов:
  - Mandelbox
  - Julia-like / Quaternion-style variant
- фрактал-специфичные parameter schemas
- корректная работа UI при смене фрактала
- пресеты, завязанные на разные фрактальные семейства

## Deliverable
Пользователь может переключаться между несколькими художественно разными фрактальными пространствами.

## Exit criteria
Фаза завершена, если:
- существует как минимум 2–3 реально рабочие формулы
- каждая из них имеет осмысленные параметры
- пресеты умеют хранить тип фрактала
- переход между режимами не ломает приложение

## Non-goals
Пока не нужны:
- 8–10 фракталов
- универсальная формульная система
- runtime formula editor

## Important note
Лучше 3 хорошо работающих фрактала, чем 8 сырых.

---

# Phase 6 — Morphing and Transitions

## Goal
Добавить плавные переходы между художественными состояниями.

## Scope
- `MorphState`
- `MorphEngine`
- `PresetInterpolator`
- interpolation policies for parameter types
- manual morph slider
- timed transition between presets
- optional reactive-triggered transitions later

## Basic use cases
- preset A → preset B
- palette transition
- lighting transition
- fractal parameter interpolation
- controlled switch of fractal type

## Deliverable
Пользователь может плавно переводить сцену из одного состояния в другое.

## Exit criteria
Фаза завершена, если:
- морфинг между близкими пресетами выглядит красиво
- архитектурно морфинг встроен как parameter layer
- пользователь может управлять переходом вручную или по времени

## Non-goals
Пока не нужны:
- идеальный morph между любыми фракталами
- physically meaningful interpolation
- super cinematic transition editor

## Important note
Морфинг — это enhancement layer.  
Он не должен ломать базовую стабильность сцены.

---

# Phase 7 — Polish and Output

## Goal
Сделать проект визуально богаче и приятнее для демонстрации.

## Scope
- soft shadows
- ambient occlusion
- better fog / atmosphere
- bloom
- vignette
- chromatic aberration, если уместно
- optional accumulation / progressive refinement
- screenshots
- frame sequence export
- performance stats / debug overlay
- profiling / quality tuning

## Deliverable
Проект выглядит заметно более “готовым” и пригодным для красивых демо-видео и скриншотов.

## Exit criteria
Фаза завершена, если:
- визуальное качество заметно выросло
- пользователь может сохранять результат
- есть базовое понимание performance bottlenecks
- проект хорошо смотрится в портфолио и demo footage

## Non-goals
Пока не нужны:
- полноценный монтажный пайплайн
- live-streaming platform features
- профессиональный NLE export stack

---

# Phase X — Future Extensions

Это не текущий обязательный план, а область возможного роста проекта.

## Possible directions

### A. Advanced audio intelligence
- better beat detection
- drop detection heuristics
- section / energy segmentation
- adaptive profiles per genre

### B. Animation systems
- LFO modulators
- automation curves
- event triggers
- camera choreography

### C. Performance mode
- hot preset switching
- fullscreen stage mode
- reduced UI mode
- keyboard / MIDI control

### D. Offline rendering
- high-quality render mode
- accumulation-based still export
- video pipe export

### E. More visual systems
- hybrid fractals
- folded fractals
- volumetric glow
- tunnel modes
- material experimentation

---

## 5. Recommended implementation order inside phases

Ниже — более приземлённый порядок выполнения, если делать руками по задачам.

### Step 1
Поднять проект:
- CMake
- GLFW
- OpenGL
- ImGui
- CUDA init

### Step 2
Сделать GPU display pipeline:
- render target
- CUDA → texture
- fullscreen output

### Step 3
Сделать один фрактал:
- Mandelbulb DE
- ray march
- normals
- basic shading

### Step 4
Сделать камеру и UI:
- movement
- mouse look
- main render params

### Step 5
Ввести Parameter System:
- registry
- values
- resolver

### Step 6
Ввести аудио:
- capture
- FFT
- AudioFeatures

### Step 7
Ввести Reactive System:
- bindings
- mapping
- controls

### Step 8
Ввести Presets:
- save/load
- preset browser

### Step 9
Добавить 2-й и 3-й фрактал

### Step 10
Добавить Morphing

### Step 11
Добавить polish-слой:
- atmosphere
- shadows
- AO
- export

---

## 6. MVP definition

Чтобы не потерять фокус, нужно отдельно зафиксировать **MVP**.

## MVP = конец Phase 3

То есть минимально жизнеспособная версия Fractal Lab — это:

- realtime 3D Mandelbulb-like rendering
- free camera
- live parameter control
- basic scene atmosphere
- audio input + FFT
- reactive bindings
- визуально приятная музыкальная реакция

Если эта версия готова, у проекта уже есть своя идентичность.

---

## 7. “Definition of done” for the project’s soul

Fractal Lab можно считать концептуально “живым”, когда выполняется следующее:

- ты реально хочешь его открыть не потому, что “надо кодить”, а потому что **хочется позалипать**
- сцена красиво реагирует на музыку
- ручное управление ощущается приятным
- есть несколько состояний, которые выглядят как отдельные миры
- проект ощущается не как лабораторка, а как авторский audiovisual tool

---

## 8. Scope control rules

Чтобы проект не расползся слишком рано, нужно придерживаться следующих правил.

### Rule 1
Нельзя добавлять новый большой subsystem, пока предыдущий этап не даёт законченного пользовательского опыта.

### Rule 2
Нельзя жертвовать удобством камеры и общей отзывчивостью ради “ещё одной красивой фичи”.

### Rule 3
Audio-reactive важнее, чем ранний morphing.

### Rule 4
Один хорошо доведённый фрактал важнее, чем много сырых формул.

### Rule 5
Пресеты важнее, чем ранний сложный post-processing.

### Rule 6
Если новая фича не делает experience более залипательным, её можно отложить.

---

## 9. Risks by phase

### Phase 0 risk
Застрять в настройке проекта и инфраструктуры.

### Phase 1 risk
Получить технически рабочий, но визуально скучный или нестабильный рендер.

### Phase 2 risk
Сделать параметры неудобными и хаотичными.

### Phase 3 risk
Сделать аудиореактивность шумной, дёрганой и не музыкальной.

### Phase 4 risk
Сохранение пресетов без полной сцены, из-за чего они будут “неполными”.

### Phase 5 risk
Добавить много фракталов, но ни один не довести до выразительного состояния.

### Phase 6 risk
Сделать морфинг математически формальным, но визуально некрасивым.

### Phase 7 risk
Начать маскировать слабую базу постэффектами.

---

## 10. Summary

Fractal Lab должен развиваться от:

**каркаса**  
→ **первого живого кадра**  
→ **управляемого explorer’а**  
→ **настоящей аудиореактивной сцены**  
→ **системы пресетов**  
→ **мультифрактального инструмента**  
→ **плавных переходов и polish-слоя**

Главный ориентир roadmap:

> на каждом этапе проект должен становиться не просто технически богаче, а более приятным, живым и залипательным для реального использования.

---

## 11. One-line roadmap statement

> Сначала — живой realtime-фрактал, потом — управляемость, потом — сильный audio-reactive core, и только после этого — расширения, морфинг и полировка.