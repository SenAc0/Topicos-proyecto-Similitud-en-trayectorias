import os
import pandas as pd

def load_geolife_plt(filepath: str) -> pd.DataFrame:
    df = pd.read_csv(filepath, skiprows=6, header=None)
    df.columns = ["lat", "lon", "unused", "alt", "timestamp", "date", "time"]
    return df[["lat", "lon", "alt", "date", "time"]]


def get_geohash(lat, lon, precision=7):
    base32 = "0123456789bcdefghjkmnpqrstuvwxyz"
    bits = []
    min_lat, max_lat = -90.0, 90.0
    min_lon, max_lon = -180.0, 180.0

    for ii in range(5 * precision):
        if ii % 2 == 0:  # longitud
            mid = (min_lon + max_lon) / 2
            if lon >= mid:
                bits.append('1')
                min_lon = mid
            else:
                bits.append('0')
                max_lon = mid
        else:  # latitud
            mid = (min_lat + max_lat) / 2
            if lat >= mid:
                bits.append('1')
                min_lat = mid
            else:
                bits.append('0')
                max_lat = mid

    bitstr = "".join(bits)
    quints = [bitstr[i*5:(i+1)*5] for i in range(precision)]
    return "".join(base32[int(q, 2)] for q in quints)



precision_list = [6, 7, 8]

input_dir = "../output_renombrado/"         # carpeta de entrada
output_base = "../output_geohash/"     # carpeta de salida

# Crear carpetas por precisión
for p in precision_list:
    os.makedirs(f"{output_base}/precision_{p}", exist_ok=True)

files = [f for f in os.listdir(input_dir) if f.endswith(".plt")]

print(f"Procesando {len(files)} trayectorias...")

count = 1

for file in files:
    full_path = os.path.join(input_dir, file)
    print(f"Procesando {file}...", count, "/", len(files))
    count += 1
    # Leer trayectoria
    df = load_geolife_plt(full_path)

    # Para cada precisión
    for p in precision_list:
        df_out = df.copy()
        df_out["geohash"] = df.apply(
            lambda row: get_geohash(row["lat"], row["lon"], precision=p),
            axis=1
        )

        out_path = f"{output_base}/precision_{p}/{file.replace('.plt', '')}.csv"
        df_out.to_csv(out_path, index=False)

print("Listo. Todas las trayectorias procesadas.")
