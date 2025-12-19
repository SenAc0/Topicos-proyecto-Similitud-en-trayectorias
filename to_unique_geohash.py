import os
import pandas as pd

# Carpetas origen y destino
input_base = r"D:\Topicos en manejo de grandes volumenes de datos\Proyecto\output_geohash"
output_base = r"D:\Topicos en manejo de grandes volumenes de datos\Proyecto\output_geohash_unique"

precision_list = [6, 7, 8]

# Crear estructura de carpetas de salida
for p in precision_list:
    os.makedirs(f"{output_base}/precision_{p}", exist_ok=True)

for p in precision_list:
    input_dir = f"{input_base}/precision_{p}"
    output_dir = f"{output_base}/precision_{p}"

    files = [f for f in os.listdir(input_dir) if f.endswith(".csv")]

    print(f"Procesando precisión {p} con {len(files)} archivos...")
    i = 1
    for file in files:
        full_path = os.path.join(input_dir, file)
        
        # Leer archivo procesado previamente
        df = pd.read_csv(full_path)

        # Mantener solo filas únicas por geohash
        df_unique = df.drop_duplicates(subset=["geohash"], keep="first")

        # Guardar con el mismo nombre
        out_path = os.path.join(output_dir, file)
        df_unique.to_csv(out_path, index=False)
        print(f"  Procesado {i}/{len(files)}")
        i += 1
print("Listo. Archivos únicos creados correctamente.")
