# Topicos-proyecto-Similitud-en-trayectorias

A continuación se enumeran la funcionalidad de cada archivo y sus requerimientos de ejecución


**minhash_with_lsh.cpp**

Calcula la similitud minhash usando lsh y minhash, y da como resultado estadistica, correlación, etc

Compilar con g++ minhash_with_lsh.cpp y /a.out

Se le consultada al usuario la cantidad de hashes, y rows, de ahi se calula la cantidad de bandas

**LCSS_compute.ipynb**

Se calcula el LCSS para las diferentes precisiones. Los resultados se copiaron y pegaron en los archivos:
  p6_lcss.txt
  p7_lcss.txt
  p8_lcss.txt

Se requiere Python, Jupyter y pandas.

**to_geohash.py y to_unique_geohash.py**

Estos codigos tienen la funcionalidad de traspasar el dataset original https://www.kaggle.com/datasets/arashnic/microsoft-geolife-gps-trajectory-dataset a sus versiones geohasheadas en la precisiones 6, 7 y 8, con todas las celdas visitadas por los puntos GPS (considerando que varias celdas se repiteb) y su version con una celda unica por trayectoria (output_unique). Se demora horas en ejectuar ya que se itera 3 veces por archivo del dataset original.

Se requiere Python y pandas.

**view_geohash.ipynb**

Tiene el objetivo de visualizar un punto (definido por defecto en el centro de Beijin) y su Geohash asociado a la precision dada (por defecto en 7)

Se requiere Python, Jupyter, pandas, follium, numpy y matplotlib.

**plot_users.ipynb**

Codigo simple para obtener la proporción de partcipación de los distintos usuarios segun su cantidad de trayectorias en el dataset

Se requiere Python, Jupyter y matplotlib.

**stadistic_geohash.ipynb**

Codigo para sacar estadistica simple de la cantidad de puntos en cada celda (media, std, mediana, max, min)

Se requiere Python, Jupyter y pandas.

**geoahash_trajectories_view.ipynb**

Codigo para hacer HTML que muestra las trayectorias con puntos GPS y celdas Geohash

Se requiere Python, Jupyter, pandas, follium, numpy y matplotlib.

**geohashing_trajectories.ipynb**

Primer acercamiento con el Geohashing, traduce una trayectoria del dataset original a Geohash (no guarda nada)

Se requiere Python, Jupyter y pandas.

**graphs.py**

Graficar en un HTML, el porcentaje k (0 a 1) de trayectorias para visualizar densidad en la capital

Se requiere Python, Jupyter, pandas, follium, numpy, pathlib y matplotlib.

