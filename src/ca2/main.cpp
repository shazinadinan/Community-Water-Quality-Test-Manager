// =====================================================================
// CAP455 Minor Project - CA2 (Persist & Search)      Units I - III
// Community Water Quality Test Manager (educational simulation)
//
// DISCLAIMER: Limits used here are CONFIGURABLE EXAMPLE values only.
// This program is NOT an official water-quality certificate. Real
// drinking-water safety needs approved standards, validated methods
// and a qualified laboratory (including microbiological tests).
//
// Build:  g++ -std=c++17 -Wall -Wextra -o ca2 main.cpp
// Run  :  ./ca2   (from the project root: ./src/ca2/ca2)
//         options: --seed N (repeatable rand())   --data FOLDER (default: data)
// =====================================================================
#include <iostream>
#include <iomanip>
#include <string>
#include <cstdlib>   // rand, srand, exit
#include <cstring>   // strncpy
#include <fstream>   // Unit III: file streams
#include <filesystem> // create the data folder
#include <cctype>
#include <ctime>
using namespace std;

// ---------- Unit I : enumerations ----------
enum SourceType { WELL = 1, BOREWELL, LAKE, RIVER, TANK };
enum SampleStatus { PENDING, TESTED, ATTENTION };

const int MAX_SOURCES = 10;
const int MAX_SAMPLES = 50;

const char* sourceTypeName(SourceType t) {
    static const char* names[] = {"?", "Well", "Borewell", "Lake", "River", "Tank"};
    return names[t];
}
const char* statusName(SampleStatus s) {
    static const char* names[] = {"PENDING", "TESTED", "NEEDS ATTENTION"};
    return names[s];
}

// ---------- Unit I : structure (all members live together) ----------
// Configurable EXAMPLE reference limits (not official thresholds).
struct Limits {
    double phMin = 6.5;
    double phMax = 8.5;
    double turbidityMax = 5.0;   // NTU (example)
    double hardnessMax = 300.0;  // mg/L as CaCO3 (example)
};
Limits gLimits;   // one shared set of limits for the program

// ---------- Unit I : recursion - hierarchical list of locations ----------
struct Location { const char* name; int parent; };
Location locations[] = {
    {"Community", -1},   // 0
    {"North Ward", 0},   // 1
    {"South Ward", 0},   // 2
    {"Lake Area", 1},    // 3
    {"Market Block", 2}, // 4
    {"Old Town", 2}      // 5
};
const int LOCATION_COUNT = sizeof(locations) / sizeof(locations[0]);

void printLocationTree(int node, int depth) {          // recursive function
    cout << string(depth * 3, ' ') << "- [" << node << "] " << locations[node].name << "\n";
    for (int i = 0; i < LOCATION_COUNT; i++)
        if (locations[i].parent == node) printLocationTree(i, depth + 1);
}
string locationPath(int node) {                         // recursion that returns a value
    if (locations[node].parent == -1) return locations[node].name;
    return locationPath(locations[node].parent) + " > " + locations[node].name;
}

// ---------- Unit I : function overloading ----------
void banner(const string& title) { cout << "\n=== " << title << " ===\n"; }
void banner(const string& title, int width) {
    cout << "\n" << string(width, '=') << "\n " << title << "\n" << string(width, '=') << "\n";
}

// ---------- Safe input helpers (no infinite loops: stop on end-of-input) ----------
void inputEnded() { cout << "\nInput ended. Exiting.\n"; exit(0); }

int readInt(const string& prompt, int lo, int hi) {
    int v;
    while (true) {
        cout << prompt;
        if (cin >> v && v >= lo && v <= hi) { cin.ignore(10000, '\n'); return v; }
        if (cin.eof()) inputEnded();
        cin.clear(); cin.ignore(10000, '\n');
        cout << "  Invalid. Enter a whole number from " << lo << " to " << hi << ".\n";
    }
}
double readDouble(const string& prompt, double lo, double hi) {
    double v;
    while (true) {
        cout << prompt;
        if (cin >> v && v >= lo && v <= hi) { cin.ignore(10000, '\n'); return v; }
        if (cin.eof()) inputEnded();
        cin.clear(); cin.ignore(10000, '\n');
        cout << "  Invalid. Enter a number from " << lo << " to " << hi << ".\n";
    }
}
string readLine(const string& prompt) {
    string s;
    while (true) {
        cout << prompt;
        getline(cin, s);
        if (cin.eof() && s.empty()) inputEnded();
        if (!s.empty()) return s;
        cout << "  Text cannot be empty.\n";
    }
}
bool isLeap(int y) { return (y % 4 == 0 && y % 100 != 0) || y % 400 == 0; }
bool validDate(const string& d) {                       // YYYY-MM-DD
    if (d.size() != 10 || d[4] != '-' || d[7] != '-') return false;
    for (int i = 0; i < 10; i++) if (i != 4 && i != 7 && !isdigit((unsigned char)d[i])) return false;
    int y = stoi(d.substr(0, 4)), m = stoi(d.substr(5, 2)), day = stoi(d.substr(8, 2));
    int dim[] = {31, isLeap(y) ? 29 : 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    return y >= 2000 && y <= 2100 && m >= 1 && m <= 12 && day >= 1 && day <= dim[m - 1];
}
string readDate(const string& prompt) {
    while (true) {
        string d = readLine(prompt);
        if (validDate(d)) return d;
        cout << "  Invalid date. Use YYYY-MM-DD (e.g. 2026-09-15).\n";
    }
}

// =====================================================================
// Class WaterSource  (Unit I: classes & objects, inline / non-inline)
// =====================================================================
class WaterSource {
    int id = 0;
    string name;
    SourceType type = WELL;
    int locationIndex = 0;
public:
    void create(int i, const string& n, SourceType t, int loc) {
        id = i; name = n; type = t; locationIndex = loc;
    }
    // inline member functions (defined inside the class)
    int getId() const { return id; }
    const string& getName() const { return name; }
    SourceType getType() const { return type; }
    // non-inline member function (declared here, defined outside)
    void display() const;
};
void WaterSource::display() const {
    cout << setw(4) << id << " | " << left << setw(14) << name << right
         << " | " << setw(8) << sourceTypeName(type) << " | " << locationPath(locationIndex) << "\n";
}

// =====================================================================
// Class WaterSample  (entity class: static, inline, friend, this, string)
// Key = sampleId (must be unique)
// =====================================================================
// Unit III: a STRUCTURE used for file records. Fixed-size, plain data only
// (char arrays, not std::string) so it is safe to write to a binary file.
struct SampleRecord {
    int sampleId;
    int sourceId;
    char date[11];          // "YYYY-MM-DD" + '\0'
    double ph, turbidity, hardness;
    int status;
    int simulated;
};
struct SourceRecord {
    int id;
    char name[32];
    int type;
    int locationIndex;
};

class WaterSample {
    int sampleId = 0;
    int sourceId = 0;
    string collectionDate;
    double ph = 7.0, turbidity = 0.0, hardness = 0.0;
    SampleStatus status = PENDING;
    bool simulated = false;           // true when values came from rand()
    static int totalSamples;          // samples registered (static data member)
    static int liveObjects;           // objects currently alive (constructors/destructor)
public:
    static int getTotalSamples() { return totalSamples; }   // static member function
    static void setTotal(int n) { totalSamples = n; }       // used after loading a file
    static int getLiveObjects() { return liveObjects; }

    // ---- Unit III: constructors ----
    WaterSample() { liveObjects++; }                         // default constructor
    // parameterized constructor + default arguments + initializer list
    WaterSample(int id, int srcId = 0, const string& date = "2000-01-01",
                double p = 7.0, double t = 0.0, double h = 0.0)
        : sampleId(id), sourceId(srcId), collectionDate(date), ph(p), turbidity(t), hardness(h) {
        totalSamples++;                                      // a NEW sample was registered
        liveObjects++;
    }
    // copy constructor (copies every member; does not count as a new sample)
    WaterSample(const WaterSample& o)
        : sampleId(o.sampleId), sourceId(o.sourceId), collectionDate(o.collectionDate),
          ph(o.ph), turbidity(o.turbidity), hardness(o.hardness),
          status(o.status), simulated(o.simulated) { liveObjects++; }
    explicit WaterSample(const SampleRecord& r);             // build an object from a file record
    WaterSample& operator=(const WaterSample&) = default;
    ~WaterSample() { liveObjects--; }                        // destructor

    // 'this' pointer: each setter returns *this so calls can be chained
    WaterSample& setReadings(double p, double t, double h, bool isSim) {
        this->ph = p; this->turbidity = t; this->hardness = h; this->simulated = isSim;
        status = TESTED;
        evaluate();
        return *this;
    }
    // inline getters
    int getId() const { return sampleId; }
    int getSourceId() const { return sourceId; }
    double getPh() const { return ph; }
    double getTurbidity() const { return turbidity; }
    double getHardness() const { return hardness; }
    SampleStatus getStatus() const { return status; }
    bool isSimulated() const { return simulated; }

    // non-inline members (defined below)
    void evaluate();
    void display() const;
    void displayFindings() const;
    SampleRecord toRecord() const;                           // class -> structure for the file

    friend class SampleAuditor;                          // friend class
    friend double combinedScore(const WaterSample& s);   // friend function
};
// Unit I: FRIEND CLASS - every member function of SampleAuditor may read WaterSample's private data
class SampleAuditor {
public:
    static string rawSummary(const WaterSample& s) {
        return "audit: id=" + to_string(s.sampleId) + " source=" + to_string(s.sourceId) +
               " status=" + statusName(s.status) + (s.simulated ? " origin=simulated" : " origin=manual");
    }
};
int WaterSample::totalSamples = 0;   // static members are defined once outside the class
int WaterSample::liveObjects = 0;

WaterSample::WaterSample(const SampleRecord& r)
    : sampleId(r.sampleId), sourceId(r.sourceId), collectionDate(r.date),
      ph(r.ph), turbidity(r.turbidity), hardness(r.hardness),
      status((SampleStatus)r.status), simulated(r.simulated != 0) { liveObjects++; }
SampleRecord WaterSample::toRecord() const {
    SampleRecord r;
    memset(&r, 0, sizeof r);                                // clear padding so files are repeatable
    r.sampleId = sampleId; r.sourceId = sourceId;
    strncpy(r.date, collectionDate.c_str(), sizeof r.date - 1);
    r.ph = ph; r.turbidity = turbidity; r.hardness = hardness;
    r.status = status; r.simulated = simulated ? 1 : 0;
    return r;
}

void WaterSample::evaluate() {
    bool bad = ph < gLimits.phMin || ph > gLimits.phMax ||
               turbidity > gLimits.turbidityMax || hardness > gLimits.hardnessMax;
    status = bad ? ATTENTION : TESTED;
}
void WaterSample::display() const {
    cout << setw(6) << sampleId << " | src " << setw(3) << sourceId << " | " << collectionDate
         << " | pH " << fixed << setprecision(1) << setw(4) << ph
         << " | Turb " << setw(5) << turbidity << " | Hard " << setw(6) << setprecision(0) << hardness
         << " | " << statusName(status) << (simulated ? " (simulated)" : "") << "\n";
}
void WaterSample::displayFindings() const {
    cout << "Sample " << sampleId << " collected " << collectionDate
         << (simulated ? "  [SIMULATED DATA]" : "  [MANUAL DATA]") << "\n" << fixed << setprecision(2);
    cout << "  pH        " << ph << "  (example range " << gLimits.phMin << "-" << gLimits.phMax << ") -> "
         << ((ph < gLimits.phMin || ph > gLimits.phMax) ? "REQUIRES ATTENTION" : "within example limit") << "\n";
    cout << "  Turbidity " << turbidity << "  (example max " << gLimits.turbidityMax << " NTU) -> "
         << (turbidity > gLimits.turbidityMax ? "REQUIRES ATTENTION" : "within example limit") << "\n";
    cout << "  Hardness  " << hardness << "  (example max " << gLimits.hardnessMax << " mg/L) -> "
         << (hardness > gLimits.hardnessMax ? "REQUIRES ATTENTION" : "within example limit") << "\n";
    cout << "  NOTE: these 3 parameters cannot show microbiological safety.\n";
}

// friend function: reads private data of WaterSample to compute a 0-100 score
double combinedScore(const WaterSample& s) {
    double score = 100.0;
    if (s.ph < gLimits.phMin) score -= (gLimits.phMin - s.ph) * 15;
    if (s.ph > gLimits.phMax) score -= (s.ph - gLimits.phMax) * 15;
    if (s.turbidity > gLimits.turbidityMax) score -= (s.turbidity - gLimits.turbidityMax) * 6;
    if (s.hardness > gLimits.hardnessMax) score -= (s.hardness - gLimits.hardnessMax) / 10.0;
    return score < 0 ? 0 : score;
}

// =====================================================================
// Class SummaryTable : multidimensional array INSIDE a class (Unit II)
// =====================================================================
class SummaryTable {
    double sum[6][3] = {};   // [source type][pH, turbidity, hardness]
    int count[6] = {};
public:
    void add(SourceType t, const WaterSample& s) {
        sum[t][0] += s.getPh(); sum[t][1] += s.getTurbidity(); sum[t][2] += s.getHardness();
        count[t]++;
    }
    void print() const {
        cout << left << setw(10) << "Type" << right << setw(5) << "N" << setw(8) << "pH"
             << setw(10) << "Turb" << setw(10) << "Hard" << "\n" << fixed << setprecision(2);
        for (int t = WELL; t <= TANK; t++) {
            if (count[t] == 0) continue;
            cout << left << setw(10) << sourceTypeName((SourceType)t) << right << setw(5) << count[t];
            for (int p = 0; p < 3; p++) cout << setw(p == 0 ? 8 : 10) << sum[t][p] / count[t];
            cout << "\n";
        }
    }
};

// ---------- Program data (Unit II: arrays of objects) ----------
WaterSource sources[MAX_SOURCES];
int sourceCount = 0;
WaterSample samples[MAX_SAMPLES];
int sampleCount = 0;

// ---------- Unit II: pointers used in the real application ----------
WaterSample* findSample(int id) {               // returns pointer or nullptr
    for (int i = 0; i < sampleCount; i++)
        if (samples[i].getId() == id) return &samples[i];
    return nullptr;
}
WaterSource* findSource(int id) {
    for (int i = 0; i < sourceCount; i++)
        if (sources[i].getId() == id) return &sources[i];
    return nullptr;
}

// =====================================================================
// Unit III: FILE LAYER  (binary fixed-size records + text log)
//   samples.dat / sources.dat : binary, one fixed-size record after another
//   log.txt                   : text, appended with ios::app
// Records are located with seekg()/seekp() (random access).
// Limitation: raw struct dumps are tied to this compiler/CPU layout, so
// the .dat files are for THIS program only (CA4 adds a portable text format).
// =====================================================================
string gDir = "data";
string samplesPath() { return gDir + "/samples.dat"; }
string sourcesPath() { return gDir + "/sources.dat"; }
string logPath()     { return gDir + "/log.txt"; }

void logEvent(const string& msg) {                      // text file, append mode
    ofstream f(logPath(), ios::app);
    if (f) f << msg << "\n";
}
bool appendSampleRecord(const SampleRecord& r) {        // binary, append mode
    ofstream f(samplesPath(), ios::binary | ios::app);
    if (!f) { cerr << "  File error: cannot open " << samplesPath() << "\n"; return false; }
    f.write(reinterpret_cast<const char*>(&r), sizeof r);
    return f.good();
}
bool appendSourceRecord(const WaterSource& s, int locationIndex) {
    SourceRecord r; memset(&r, 0, sizeof r);
    r.id = s.getId(); strncpy(r.name, s.getName().c_str(), sizeof r.name - 1);
    r.type = s.getType(); r.locationIndex = locationIndex;
    ofstream f(sourcesPath(), ios::binary | ios::app);
    if (!f) { cerr << "  File error: cannot open " << sourcesPath() << "\n"; return false; }
    f.write(reinterpret_cast<const char*>(&r), sizeof r);
    return f.good();
}
int recordCountInFile(const string& path, size_t recSize) {
    ifstream f(path, ios::binary);
    if (!f) return 0;
    f.seekg(0, ios::end);                               // jump to end to learn the size
    return (int)(f.tellg() / (long long)recSize);
}
// Sequential read of whole files at program start
void loadAll() {
    ifstream fs(sourcesPath(), ios::binary);
    SourceRecord sr;
    while (fs && fs.read(reinterpret_cast<char*>(&sr), sizeof sr)) {
        if (sourceCount >= MAX_SOURCES || findSource(sr.id)) continue;
        sources[sourceCount++].create(sr.id, sr.name, (SourceType)sr.type, sr.locationIndex);
    }
    ifstream f(samplesPath(), ios::binary);
    SampleRecord r;
    while (f && f.read(reinterpret_cast<char*>(&r), sizeof r)) {
        if (sampleCount >= MAX_SAMPLES || findSample(r.sampleId)) continue;
        samples[sampleCount] = WaterSample(r);
        samples[sampleCount].evaluate();
        sampleCount++;
    }
    WaterSample::setTotal(sampleCount);
    cout << "Loaded " << sourceCount << " source(s) and " << sampleCount << " sample(s) from '" << gDir << "'.\n";
}
// Random-access search: jump (seekg) straight to each record position
int findIndexInFile(int id, SampleRecord& out) {
    ifstream f(samplesPath(), ios::binary);
    if (!f) return -1;
    int n = recordCountInFile(samplesPath(), sizeof(SampleRecord));
    for (int i = 0; i < n; i++) {
        f.seekg((long long)i * sizeof(SampleRecord), ios::beg);
        f.read(reinterpret_cast<char*>(&out), sizeof out);
        if (f && out.sampleId == id) return i;
    }
    return -1;
}
bool overwriteRecordInFile(int index, const SampleRecord& r) {      // random-access update
    fstream f(samplesPath(), ios::in | ios::out | ios::binary);
    if (!f) { cerr << "  File error: cannot open " << samplesPath() << "\n"; return false; }
    f.seekp((long long)index * sizeof(SampleRecord), ios::beg);      // jump to the record
    f.write(reinterpret_cast<const char*>(&r), sizeof r);
    f.flush();
    return f.good();
}
bool rewriteAllSamples() {                                           // truncate + write all
    ofstream f(samplesPath(), ios::binary | ios::trunc);
    if (!f) return false;
    for (int i = 0; i < sampleCount; i++) { SampleRecord r = samples[i].toRecord(); f.write(reinterpret_cast<const char*>(&r), sizeof r); }
    return f.good();
}

// ---------- Registering ----------
void registerSource() {
    banner("Register water source");
    if (sourceCount >= MAX_SOURCES) { cout << "Source limit reached.\n"; return; }
    int id = readInt("Source ID (1-999): ", 1, 999);
    if (findSource(id)) { cout << "  Duplicate source ID. Not saved.\n"; return; }
    string name = readLine("Source name: ");
    cout << "Type: 1 Well, 2 Borewell, 3 Lake, 4 River, 5 Tank\n";
    int t = readInt("Choose type: ", 1, 5);
    cout << "Locations:\n"; printLocationTree(0, 1);
    int loc = readInt("Location number: ", 0, LOCATION_COUNT - 1);
    sources[sourceCount].create(id, name, (SourceType)t, loc);
    appendSourceRecord(sources[sourceCount], loc);          // saved immediately
    sourceCount++;
    logEvent("source " + to_string(id) + " registered");
    cout << "Source registered and saved.\n";
}
void listSources() {
    banner("Registered sources");
    if (sourceCount == 0) { cout << "None yet.\n"; return; }
    for (int i = 0; i < sourceCount; i++) sources[i].display();
}

// Unit I: call by value / address / reference used for real work
void clampToRange(double* value, double lo, double hi) {      // call by ADDRESS
    if (*value < lo) *value = lo;
    if (*value > hi) *value = hi;
}
void simulateReadings(int scenario, double& ph, double& turb, double& hard) {   // call by REFERENCE
    switch (scenario) {
        case 1: ph = 6.8 + (rand() % 15) / 10.0; turb = (rand() % 40) / 10.0; hard = 80 + rand() % 200; break;   // typical
        case 2: ph = 6.5 + (rand() % 20) / 10.0; turb = 5 + (rand() % 60) / 10.0; hard = 100 + rand() % 150; break; // after rain
        case 3: ph = 7.0 + (rand() % 20) / 10.0; turb = (rand() % 30) / 10.0; hard = 300 + rand() % 200; break;  // hard water
        default: ph = 4.0 + (rand() % 90) / 10.0; turb = (rand() % 100) / 10.0; hard = 20 + rand() % 480; break; // fully random
    }
    clampToRange(&ph, 0, 14);
}

void addSample(bool simulate) {
    banner(simulate ? "New sample (SIMULATED readings)" : "New sample (manual readings)");
    if (sourceCount == 0) { cout << "Register a source first.\n"; return; }
    if (sampleCount >= MAX_SAMPLES) { cout << "Sample limit reached.\n"; return; }
    int id = readInt("Sample ID (1-99999): ", 1, 99999);
    if (findSample(id)) { cout << "  Duplicate sample ID. Not saved.\n"; return; }
    int src = readInt("Source ID: ", 1, 999);
    if (!findSource(src)) { cout << "  Source not found.\n"; return; }
    string date = readDate("Collection date (YYYY-MM-DD): ");
    double ph, turb, hard;
    if (simulate) {
        cout << "Scenario: 1 Typical, 2 After heavy rain, 3 Hard-water area, 4 Fully random\n";
        int sc = readInt("Choose scenario: ", 1, 4);
        simulateReadings(sc, ph, turb, hard);
        cout << "  (values generated with rand(); they are NOT measurements)\n";
    } else {
        ph = readDouble("pH (0-14): ", 0, 14);
        turb = readDouble("Turbidity NTU (0-1000): ", 0, 1000);
        hard = readDouble("Hardness mg/L (0-2000): ", 0, 2000);
    }
    samples[sampleCount] = WaterSample(id, src, date);            // parameterized constructor
    samples[sampleCount].setReadings(ph, turb, hard, simulate);
    appendSampleRecord(samples[sampleCount].toRecord());            // saved immediately
    logEvent("sample " + to_string(id) + (simulate ? " simulated" : " manual"));
    samples[sampleCount].displayFindings();
    cout << "Combined example score: " << fixed << setprecision(1)
         << combinedScore(samples[sampleCount]) << " / 100\n";
    sampleCount++;
}

void viewAll() {
    banner("All samples");
    if (sampleCount == 0) { cout << "None yet.\n"; return; }
    for (int i = 0; i < sampleCount; i++) samples[i].display();
}
void viewOne() {
    int id = readInt("Sample ID: ", 1, 99999);
    WaterSample* p = findSample(id);
    if (!p) { cout << "  Sample not found.\n"; return; }
    p->displayFindings();
    cout << SampleAuditor::rawSummary(*p) << "\n";
    cout << "Combined example score: " << fixed << setprecision(1) << combinedScore(*p) << " / 100\n";
}
void summaryReport() {
    banner("Summary report");
    int stat[3][6] = {};   // multidimensional array in main-level code: [status][source type]
    SummaryTable table;
    for (int i = 0; i < sampleCount; i++) {
        WaterSource* src = findSource(samples[i].getSourceId());
        SourceType t = src ? src->getType() : WELL;
        stat[samples[i].getStatus()][t]++;
        table.add(t, samples[i]);
    }
    cout << "Samples created so far (static counter): " << WaterSample::getTotalSamples() << "\n";
    cout << "Sample objects alive in memory (static counter): " << WaterSample::getLiveObjects() << "\n";
    int needs = 0;
    for (int t = WELL; t <= TANK; t++) needs += stat[ATTENTION][t];
    cout << "Readings requiring attention: " << needs << " of " << sampleCount << "\n";
    if (sampleCount) { cout << "\nAverages by source type:\n"; table.print(); }
    cout << "\nReminder: educational simulation, not a drinking-water certificate.\n";
}
void editLimits() {
    banner("Edit example reference limits");
    gLimits.phMin = readDouble("pH minimum (0-14): ", 0, 14);
    gLimits.phMax = readDouble("pH maximum (0-14): ", gLimits.phMin, 14);
    gLimits.turbidityMax = readDouble("Turbidity max NTU (0-1000): ", 0, 1000);
    gLimits.hardnessMax = readDouble("Hardness max mg/L (0-2000): ", 0, 2000);
    for (int i = 0; i < sampleCount; i++) samples[i].evaluate();   // re-assess old samples
    cout << "Limits updated; existing samples re-assessed.\n";
}

void searchSampleInFile() {
    banner("Search in file (seekg)");
    int id = readInt("Sample ID to find in samples.dat: ", 1, 99999);
    SampleRecord r;
    int idx = findIndexInFile(id, r);
    if (idx < 0) { cout << "  Not found in file.\n"; return; }
    WaterSample w(r);
    cout << "Found at record #" << idx << " (byte offset " << (long long)idx * sizeof(SampleRecord) << "):\n";
    w.display();
}
void updateSample() {
    banner("Update sample readings (seekp)");
    int id = readInt("Sample ID to update: ", 1, 99999);
    WaterSample* p = findSample(id);
    if (!p) { cout << "  Sample not found.\n"; return; }
    p->displayFindings();
    char c = readLine("Replace the readings? (y/n): ")[0];
    if (c != 'y' && c != 'Y') { cout << "Update cancelled.\n"; return; }
    double ph = readDouble("New pH (0-14): ", 0, 14);
    double tu = readDouble("New turbidity (0-1000): ", 0, 1000);
    double ha = readDouble("New hardness (0-2000): ", 0, 2000);
    p->setReadings(ph, tu, ha, false);
    SampleRecord tmp;
    int idx = findIndexInFile(id, tmp);
    if (idx < 0) { cout << "  Record missing in file; memory updated only.\n"; return; }
    if (overwriteRecordInFile(idx, p->toRecord())) { cout << "Record #" << idx << " overwritten in file.\n"; logEvent("sample " + to_string(id) + " updated"); }
}
void saveAllNow() {
    if (rewriteAllSamples()) cout << "Saved " << sampleCount << " sample(s) to " << samplesPath() << ".\n";
    else cout << "  File error while saving.\n";
}
void exportTextReport() {
    string path = gDir + "/report.txt";
    ofstream out(path);
    if (!out) { cout << "  File error: cannot write " << path << "\n"; return; }
    out << "Water Quality Test Manager - text report (educational, example limits)\n";
    for (int i = 0; i < sampleCount; i++)
        out << samples[i].getId() << "," << samples[i].getSourceId() << "," << statusName(samples[i].getStatus()) << "\n";
    cout << "Text report written to " << path << ".\n";
}

void menu() {
    cout << "\n========== Community Water Quality Test Manager (CA2) ==========\n"
         << " 1 Register water source      10 Save all samples\n"
         << " 2 List sources               11 Search in file (seekg)\n"
         << " 3 New sample (manual)        12 Update sample (seekp)\n"
         << " 4 New sample (simulated)     13 Export text report\n"
         << " 5 View all samples           14 Exit\n"
         << " 6 View one sample (findings)\n"
         << " 7 Summary report\n"
         << " 8 Edit example limits\n"
         << " 9 Location tree (recursion)\n";
}

int main(int argc, char* argv[]) {
    unsigned seed = (unsigned)time(nullptr);
    for (int i = 1; i + 1 < argc; i += 2) {
        if (string(argv[i]) == "--seed") seed = (unsigned)atoi(argv[i + 1]);
        else if (string(argv[i]) == "--data") gDir = argv[i + 1];
    }
    srand(seed);
    { std::error_code ec; std::filesystem::create_directories(gDir, ec);
      if (ec) cerr << "Warning: cannot create data folder '" << gDir << "': " << ec.message() << "\n"; }
    loadAll();                       // Unit III: data survives a restart
    cout << "Educational simulation. Example limits only - NOT official safety certification.\n";
    int choice;
    do {
        menu();
        choice = readInt("Choose: ", 1, 14);
        switch (choice) {
            case 1: registerSource(); break;
            case 2: listSources(); break;
            case 3: addSample(false); break;
            case 4: addSample(true); break;
            case 5: viewAll(); break;
            case 6: viewOne(); break;
            case 7: summaryReport(); break;
            case 8: editLimits(); break;
            case 9: banner("Location hierarchy", 40); printLocationTree(0, 0); break;
            case 10: saveAllNow(); break;
            case 11: searchSampleInFile(); break;
            case 12: updateSample(); break;
            case 13: exportTextReport(); break;
            case 14: cout << "\nThank you for using the system. Goodbye!\n"; break;
        }
    } while (choice != 14);
    return 0;
}
