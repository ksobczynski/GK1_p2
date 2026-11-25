# GK1_p2 – Powierzchnie Béziera

## Uruchomienie

Wymagania: CMake ≥ 3.16, kompilator C++17, Qt 6 (Widgets).

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
./build/p2                        # domyślnie data/surfaces.txt
./build/p2 -f data/surfaces2.txt  # inny plik
```

- Jeśli CMake nie znajduje Qt, dodaj `-DCMAKE_PREFIX_PATH=<katalog Qt>`, np. `/opt/homebrew/opt/qt` (macOS), `C:/Qt/6.x.x/msvc2022_64` (Windows).
- Windows: plik wykonywalny to `build\Release\p2.exe`; przed uruchomieniem `windeployqt` albo katalog `bin` Qt w `PATH`.
- Program uruchamiamy z katalogu projektu – ścieżki do `data/` są względne.

## Dane wejściowe

Plik tekstowy, jeden punkt kontrolny `x y z` na wiersz. Każde kolejne 16 wierszy to jedna powierzchnia (siatka 4×4). Wymagane są co najmniej 2 powierzchnie; błędny plik kończy program komunikatem.

## Jak to działa

| Etap | Plik | Opis |
|---|---|---|
| Wczytanie | `model/SurfaceLoader.cpp` | 16 punktów → `BezierSurface`, punkt k → `P[k/4][k%4]` |
| Triangulacja | `model/BezierSurface.cpp` | siatka (n+1)×(n+1) w (u,v) ∈ [0,1]², 2n² trójkątów na powierzchnię |
| Obrót | `model/Scene.cpp` | `Pr = R·P`, `Pur = R·Pu`, `Pvr = R·Pv`, `Nr = normalize(R·N)` |
| Wypełnianie | `render/Rasterizer.h` | sortowanie wierzchołków po y, scanline, Z-bufor |
| Cieniowanie | `render/Lighting.h`, `ui/CanvasWidget.cpp` | Phong w każdym pikselu |
| Światło | `model/LightCurve.cpp` | ruch po zamkniętej krzywej Béziera na płaszczyźnie z = const |

### Powierzchnia
- P(u,v) = Σᵢ Σⱼ Vᵢⱼ · Bᵢ(u) · Bⱼ(v), baza Bernsteina 3. stopnia.
- Pu, Pv – pochodne cząstkowe (różniczkowana baza), N = normalize(Pu × Pv).
- Każdy wierzchołek przechowuje P, Pu, Pv, N (przed i po obrocie) oraz (u, v).

### Obrót i rzut
- Orientacja to macierz R, składana narastająco: R_nowa = Rx(α)·Ry(β)·R_aktualna.
- α = 0.5°·dy, β = 0.5°·dx, gdzie (dx, dy) to przesunięcie kursora od poprzedniej pozycji. Mnożenie z lewej → obrót względem osi ekranu.
- Rzut prostokątny: ekran = (x, y), z służy jako głębokość. Obserwator na +z, V = [0,0,1].

### Wypełnianie trójkąta
- Wierzchołki sortowane po y (A, B, C); w każdym wierszu przecięcie z krawędzią AC i z AB lub BC.
- Piksel należy do trójkąta, gdy jego środek (x+0.5, y+0.5) leży wewnątrz – brak dziur i podwójnych pikseli na wspólnych krawędziach.
- Współrzędne barycentryczne z funkcji krawędzi (podwójne pole ze znakiem), w wierszu aktualizowane przyrostowo.
- Interpolowane: z, P, N, (u, v), Pu, Pv.
- Z-bufor: zostaje fragment o większym z (bliżej obserwatora).

### Oświetlenie (Phong)
```
I = kd·IL·IO·cos(N,L) + ks·IL·IO·cosᵐ(V,R),   R = 2⟨N,L⟩N − L
```
- Osobno dla R, G, B, w zakresie [0,1], ujemne cosinusy → 0, wynik obcinany do 255.
- Brak składowej otoczenia – strona odwrócona od światła jest czarna.

### Kolor obiektu IO
- Jednolity kolor albo tekstura. Piksel tekstury: (u·(W−1), v·(H−1)), najbliższy sąsiad. Domyślnie `data/texture.png`.

### Mapa normalnych
- N_t = 2·kolor/255 − 1, N = normalize(Pu·N_t.x + Pv·N_t.y + N·N_t.z), czyli M·N_t dla M = [Pu, Pv, N].
- Domyślnie `data/normalmap.png`.

### Źródło światła
- Krzywa: 4 segmenty kubiczne (12 punktów kontrolnych), zamknięta, klasy C1.
- Parametr s ∈ [0,1): segment k = ⌊4s⌋, t = 4s − k.
- Edycja: przesunięcie punktu łączenia przesuwa jego uchwyty; przesunięcie uchwytu odbija drugi symetrycznie względem punktu łączenia (zachowuje C1).
- Animacja: timer 16 ms, s += 0.002.

## Obsługa

- **Lewy przycisk + przeciąganie** – obrót sceny.
- **Przeciąganie pomarańczowego punktu** – edycja krzywej światła.

| Klawisz | Działanie |
|---|---|
| F1 | pomoc |
| R | reset obrotu |
| B / S / F | wielobok Béziera / siatka / wypełnienie |
| C / T | IO: kolor / tekstura |
| N | mapa normalnych |
| A | animacja światła |

Panel: dokładność triangulacji (1–50), kd, ks, m, kolor światła i obiektu, wczytanie tekstury i mapy normalnych, wysokość z światła.
