#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <set>
#include <random>
#include <climits>
#include <unordered_map>
#include <algorithm>
#include <cmath>
using namespace std;

class trajectory {
    int n; // number of points
    vector<double> latitude;
    vector<double> longitude;
    vector<int> alt;
    vector<string> date;
    vector<string> time;
    vector<string> geohash;

public:
    trajectory(int n){
        this->n = n;   // solo informativo
    }

    void insert(double lat, double lon, int a,
                const string& d, const string& t,
                const string& gh)
    {
        latitude.push_back(lat);
        longitude.push_back(lon);
        alt.push_back(a);
        date.push_back(d);
        time.push_back(t);
        geohash.push_back(gh);
    }

    void print() const {
        size_t sz = latitude.size();
        for(size_t i = 0; i < sz; i++){
            cout << latitude[i] << "," 
                 << longitude[i] << "," 
                 << alt[i] << "," 
                 << date[i] << "," 
                 << time[i] << "," 
                 << geohash[i] << "\n";
        }
    }

    set<string> get_geohash_set() const {
        return set<string>(geohash.begin(), geohash.end());
    }
};


trajectory read_trajectory_from_csv(const string& filename){
    ifstream file(filename);
    if(!file.is_open()){
        cerr << "Error: no se pudo abrir " << filename << "\n";
        return trajectory(0);
    }
    string line;

    // saltar header
    getline(file, line);

    trajectory traj(0);

    while(getline(file, line)){
        if(line.size() < 3) continue;

        stringstream ss(line);
        vector<string> row;
        string item;

        while(getline(ss, item, ',')){
            row.push_back(item);
        }

        if(row.size() < 6) continue;

        double lat = stod(row[0]);
        double lon = stod(row[1]);
        int a      = stoi(row[2]);
        string d   = row[3];
        string t   = row[4];
        string gh  = row[5];

        traj.insert(lat, lon, a, d, t, gh);
    }

    return traj;
}


// A y B de las funciones hash

const uint64_t PRIME = 4294967311ULL;     // primo grande

uint64_t string_hash(const string& s){
    uint64_t h = 14695981039346656037ULL;
    for(char c : s){
        h ^= (uint64_t)c;
        h *= 1099511628211ULL;
    }
    return h;
}

vector<uint64_t> gen_random_coeffs(int k, uint64_t seed = 42){
    mt19937_64 gen(seed); 
    uniform_int_distribution<uint64_t> dis(1, PRIME - 1);

    vector<uint64_t> coeffs(k);
    for(int i = 0; i < k; i++)
        coeffs[i] = dis(gen);

    return coeffs;
}

vector<uint64_t> minhash_signature(
    const set<string>& geohashes,
    const vector<uint64_t>& A,
    const vector<uint64_t>& B,
    int num_hash)
{
    vector<uint64_t> signature(num_hash, ULLONG_MAX);

    for(const auto& gh : geohashes){
        uint64_t x = string_hash(gh);

        for(int i = 0; i < num_hash; i++){
            uint64_t h = (A[i] * x + B[i]) % PRIME;
            signature[i] = min(signature[i], h);
        }
    }
    return signature;
}

double minhash_similarity(
    const vector<uint64_t>& sig1,
    const vector<uint64_t>& sig2)
{
    if(sig1.size() != sig2.size()) return 0.0;

    int equal = 0;
    for(size_t i = 0; i < sig1.size(); i++){
        if(sig1[i] == sig2[i])
            equal++;
    }
    return double(equal) / sig1.size();
}


double jaccard_exact(const set<string>& A, const set<string>& B){
    int inter = 0;
    for(const auto& x : A)
        if(B.count(x)) inter++;

    int uni = A.size() + B.size() - inter;
    return double(inter) / uni;
}


uint64_t hash_band(const vector<uint64_t>& sig, int band_id, int start, int rows){
    uint64_t h = 14695981039346656037ULL; 
    h ^= (uint64_t)band_id; 
    h *= 1099511628211ULL;
    for(int i = 0; i < rows; i++){
        h ^= sig[start + i];
        h *= 1099511628211ULL;
    }
    return h;
}

void lsh_insert(
    int traj_id,
    const vector<uint64_t>& sig,
    int rows_per_band,
    unordered_map<uint64_t, vector<int>>& buckets)
{
    int num_bands = sig.size() / rows_per_band;

    for(int b = 0; b < num_bands; b++){
        int start = b * rows_per_band;
        uint64_t key = hash_band(sig, b, start, rows_per_band);
        buckets[key].push_back(traj_id);
    }
}

set<int> lsh_candidates(
    int traj_id,
    const vector<uint64_t>& sig,
    int rows_per_band,
    unordered_map<uint64_t, vector<int>>& buckets)
{
    set<int> candidates;
    int num_bands = sig.size() / rows_per_band;

    for(int b = 0; b < num_bands; b++){
        int start = b * rows_per_band;
        uint64_t key = hash_band(sig, b, start, rows_per_band);

        for(int id : buckets[key]){
            if(id != traj_id)
                candidates.insert(id);
        }
    }
    return candidates;
}



int main(){
    int num_hash, rows_per_band;
    uint64_t seed = 42;

    cout << "N funciones hash: ";
    cin >> num_hash;
    
    cout << "Filas por banda (LSH): ";
    cin >> rows_per_band;
    
    // Validar parámetros LSH
    if(rows_per_band <= 0 || rows_per_band > num_hash){
        cerr << "Error: rows_per_band debe estar entre 1 y " << num_hash << "\n";
        return 1;
    }
    
    int num_bands = num_hash / rows_per_band;
    if(num_bands == 0){
        cerr << "Error: rows_per_band (" << rows_per_band << ") es mayor que num_hash (" << num_hash << ")\n";
        return 1;
    }
    
    if(num_hash % rows_per_band != 0){
        cerr << "Advertencia: " << num_hash << " % " << rows_per_band << " = " 
             << (num_hash % rows_per_band) << " (se perderán " 
             << (num_hash % rows_per_band) << " hashes)\n";
    }
    cout << "\n";
    cout << "Funciones hash: " << num_hash << "\n";
    cout << "Filas por banda: " << rows_per_band << "\n";
    cout << "Numero de bandas: " << num_bands << "\n";
    
    double threshold = pow(1.0 / num_bands, 1.0 / rows_per_band);



    vector<string> files;
    const string base_path = "../output_geohash_unique/precision_7/";
    
    cout << "Escaneando archivos\n";
    for(int user_id = 0; user_id <= 181; user_id++){
        int found_for_user = 0;
        for(int traj = 0; traj < 5000; traj++){
            stringstream ss;
            ss << base_path << setw(3) << setfill('0') << user_id << "_" << traj << ".csv";
            ifstream test_file(ss.str());
            if(test_file.good()){
                files.push_back(ss.str());
                found_for_user++;
                test_file.close();
            }
        }
        if(files.size() % 5000 == 0 && files.size() > 0){
            cout << "  " << files.size() << " archivos\n";
        }
    }

    if(files.empty()){
        cerr << "Error: no se encontraron archivos CSV en " << base_path << "\n";
        return 1;
    }

    cout << "Total: " << files.size() << " archivos\n";

    vector<uint64_t> A = gen_random_coeffs(num_hash, seed);
    vector<uint64_t> B = gen_random_coeffs(num_hash, seed + 1);

    unordered_map<uint64_t, vector<int>> lsh_buckets;
    vector<vector<uint64_t>> signatures;
    signatures.reserve(files.size());

    cout << "Generando firmas e insertando en LSH\n";
    int empty_geohash_count = 0;
    for(size_t idx = 0; idx < files.size(); idx++){
        trajectory t = read_trajectory_from_csv(files[idx]);
        set<string> gh_set = t.get_geohash_set();
        
        if(gh_set.empty()){
            empty_geohash_count++;
        }
        
        vector<uint64_t> sig = minhash_signature(gh_set, A, B, num_hash);
        signatures.push_back(sig);
        lsh_insert(idx, sig, rows_per_band, lsh_buckets);
        
        if((idx + 1) % 2000 == 0){
            cout << "  " << (idx + 1) << "/" << files.size() << "\n";
        }
    }


    set<pair<int,int>> candidate_pairs_set;
    
    for(size_t i = 0; i < signatures.size(); i++){
        set<int> candidates = lsh_candidates(i, signatures[i], rows_per_band, lsh_buckets);
        
        for(int j : candidates){
            if(i < j){
                candidate_pairs_set.insert({i, j});
            }
        }
        
        if((i + 1) % 2000 == 0){
            cout << "  " << (i + 1) << " consultas, " << candidate_pairs_set.size() << " pares\n";
        }
    }
    
    cout << "Pares candidatos: " << candidate_pairs_set.size() << "\n\n";

    vector<pair<int,int>> candidate_pairs(candidate_pairs_set.begin(), candidate_pairs_set.end());
    
    int num_pairs_to_evaluate = min(10000, (int)candidate_pairs.size());
    if(candidate_pairs.size() > 10000){

        mt19937_64 shuffle_gen(seed);
        shuffle(candidate_pairs.begin(), candidate_pairs.end(), shuffle_gen);
        candidate_pairs.resize(10000);
    }

    
    set<int> needed_indices;
    for(const auto& p : candidate_pairs){
        needed_indices.insert(p.first);
        needed_indices.insert(p.second);
    }
    
    cout << "Cargando " << needed_indices.size() << " trayectorias\n";
    unordered_map<int, set<string>> gh_sets;
    
    int loaded = 0;
    for(int idx : needed_indices){
        trajectory t = read_trajectory_from_csv(files[idx]);
        gh_sets[idx] = t.get_geohash_set();
        loaded++;
        
        if(loaded % 1000 == 0){
            cout << "  " << loaded << "/" << needed_indices.size() << "\n";
        }
    }
    cout << "Cargadas\n\n";

    system("mkdir results_minhash 2>nul");
    
    stringstream ss_prefix;
    ss_prefix << "results_minhash/k" << num_hash << "_b" << num_bands << "_r" << rows_per_band;
    string prefix = ss_prefix.str();
    
    cout << "Evaluando " << num_pairs_to_evaluate << " pares\n";
    vector<double> errors;
    vector<double> minhash_sims;
    vector<double> jaccard_sims;
    vector<pair<int,int>> evaluated_pairs;
    
    ofstream results_file(prefix + "_resultados_pares.csv");
    results_file << "par_i,par_j,minhash_sim,jaccard_sim,error_abs\n";
    
    ofstream correlation_data(prefix + "_datos_correlacion.csv");
    correlation_data << "minhash,jaccard\n";
    
    for(size_t p = 0; p < candidate_pairs.size(); p++){
        int i = candidate_pairs[p].first;
        int j = candidate_pairs[p].second;
        
        // aca se hace el calculo de la similitud y jaccard exacto con los candidatos obtenidos por el lsh

        double sim_minhash = minhash_similarity(signatures[i], signatures[j]);
        double sim_jaccard = jaccard_exact(gh_sets[i], gh_sets[j]); // esto es calculado sobre los sets completos de geohash, no sobre las signatures
        double error = abs(sim_minhash - sim_jaccard);
        
        errors.push_back(error);
        minhash_sims.push_back(sim_minhash);
        jaccard_sims.push_back(sim_jaccard);
        evaluated_pairs.push_back({i, j});
        
        results_file << i << "," << j << "," 
                    << sim_minhash << "," << sim_jaccard << "," 
                    << error << "\n";
        
        correlation_data << sim_minhash << "," << sim_jaccard << "\n";

        if((p + 1) % 2000 == 0){
            cout << "  " << (p + 1) << "/" << num_pairs_to_evaluate << "\n";
        }
    }
    results_file.close();
    correlation_data.close();
    cout << "Guardado: " << prefix << "_resultados_pares.csv\n";
    cout << "Guardado: " << prefix << "_datos_correlacion.csv\n\n";

    // Calcular estadísticas
    double mean_error = 0.0, max_error = 0.0;
    double mean_minhash = 0.0, mean_jaccard = 0.0;
    
    for(size_t i = 0; i < errors.size(); i++){
        mean_error += errors[i];
        mean_minhash += minhash_sims[i];
        mean_jaccard += jaccard_sims[i];
        max_error = max(max_error, errors[i]);
    }
    mean_error /= num_pairs_to_evaluate;
    mean_minhash /= num_pairs_to_evaluate;
    mean_jaccard /= num_pairs_to_evaluate;

    // Calcular correlación de Pearson
    double sum_xy = 0.0, sum_x2 = 0.0, sum_y2 = 0.0;
    for(size_t i = 0; i < errors.size(); i++){
        double dx = minhash_sims[i] - mean_minhash;
        double dy = jaccard_sims[i] - mean_jaccard;
        sum_xy += dx * dy;
        sum_x2 += dx * dx;
        sum_y2 += dy * dy;
    }
    double correlation = sum_xy / sqrt(sum_x2 * sum_y2);

    int true_positives = 0;
    int false_positives = 0;
    
    for(size_t i = 0; i < jaccard_sims.size(); i++){
        if(jaccard_sims[i] >= threshold){
            true_positives++;
        } else {
            false_positives++;
        }
    }
    

    // imprimir resultados y guardlos de aqui en adelante

    double precision = (double)true_positives / num_pairs_to_evaluate;
    

    cout << "Trayectorias procesadas: " << files.size() << "\n";
    cout << "Pares candidatos: " << candidate_pairs_set.size() << "\n";
    cout << "Pares evaluados: " << num_pairs_to_evaluate << "\n";
    cout << "Trayectorias en memoria: " << needed_indices.size() << "\n\n";
    cout << "Error abs promedio: " << mean_error << "\n";
    cout << "Error abs maximo: " << max_error << "\n";
    cout << "MinHash promedio: " << mean_minhash << "\n";
    cout << "Jaccard promedio: " << mean_jaccard << "\n";
    cout << "Correlacion Pearson: " << correlation << "\n\n";
    cout << "True Positives: " << true_positives << " (" << (precision*100) << "%)\n";
    cout << "False Positives: " << false_positives << " (" << ((1-precision)*100) << "%)\n";
    
    ofstream metrics_file(prefix + "_metricas_finales.txt");
    metrics_file << "Configuracion LSH:\n";
    metrics_file << "Funciones hash: " << num_hash << "\n";
    metrics_file << "Filas por banda: " << rows_per_band << "\n";
    metrics_file << "Numero de bandas: " << num_bands << "\n";
    metrics_file << "Umbral teorico: " << threshold << "\n";
    metrics_file << "Seed: " << seed << "\n\n";
    
    metrics_file << "Dataset:\n";
    metrics_file << "Total trayectorias: " << files.size() << "\n";
    metrics_file << "Pares candidatos: " << candidate_pairs_set.size() << "\n";
    metrics_file << "Pares evaluados: " << num_pairs_to_evaluate << "\n";
    metrics_file << "Trayectorias memoria: " << needed_indices.size() << "\n\n";
    
    metrics_file << "Precision MinHash:\n";
    metrics_file << "Error abs promedio: " << mean_error << "\n";
    metrics_file << "Error abs maximo: " << max_error << "\n";
    metrics_file << "MinHash promedio: " << mean_minhash << "\n";
    metrics_file << "Jaccard promedio: " << mean_jaccard << "\n";
    metrics_file << "Correlacion Pearson: " << correlation << "\n";
    metrics_file.close();
    cout << "Guardado: " << prefix << "_metricas_finales.txt\n";
    
    ofstream classification_file(prefix + "_clasificacion_lsh.txt");
    classification_file << "Umbral LSH: " << threshold << "\n\n";
    classification_file << "True Positive: Jaccard >= " << threshold << "\n";
    classification_file << "False Positive: Jaccard < " << threshold << "\n\n";
    
    classification_file << "Resultados:\n";
    classification_file << "Total evaluados: " << num_pairs_to_evaluate << "\n";
    classification_file << "True Positives: " << true_positives << " (" << (precision*100) << "%)\n";
    classification_file << "False Positives: " << false_positives << " (" << ((1-precision)*100) << "%)\n\n";
    
    classification_file << "Precision: " << precision << "\n";
    classification_file << "FP rate: " << (1-precision) << "\n\n";
    
    int bins[10] = {0};
    for(double sim : jaccard_sims){
        int bin = min(9, (int)(sim * 10));
        bins[bin]++;
    }
    
    classification_file << "Distribucion Jaccard:\n";
    for(int i = 0; i < 10; i++){
        double lower = i * 0.1;
        double upper = (i + 1) * 0.1;
        classification_file << "[" << lower << "-" << upper << "): " << bins[i] << "\n";
    }
    classification_file.close();
    cout << "Guardado: " << prefix << "_clasificacion_lsh.txt\n";

    return 0;
}


