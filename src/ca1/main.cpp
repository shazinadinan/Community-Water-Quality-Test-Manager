// =====================================================================
// CAP455 Minor Project - CA1 (Design & Create)       Units I - II
// Community Water Quality Test Manager (educational simulation)
//
// DISCLAIMER: Limits used here are CONFIGURABLE EXAMPLE values only.
// This program is NOT an official water-quality certificate. Real
// drinking-water safety needs approved standards, validated methods
// and a qualified laboratory (including microbiological tests).

#include <iostream>
#include <iomanip>
#include <string>
#include <cstdlib>   // rand, srand, exit
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
class WaterSample {
    int sampleId = 0;
    int sourceId = 0;
    string collectionDate;
    double ph = 7.0, turbidity = 0.0, hardness = 0.0;
    SampleStatus status = PENDING;
    bool simulated = false;           // true when values came from rand()
    static int totalSamples;          // Unit I: static data member
public:
    static int getTotalSamples() { return totalSamples; }   // static member function

    // 'this' pointer: each setter returns *this so calls can be chained
    WaterSample& setBasic(int id, int srcId, const string& date) {
        this->sampleId = id; this->sourceId = srcId; this->collectionDate = date;
        totalSamples++;               // one more sample created
        return *this;
    }
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

    friend class SampleAuditor;                          // friend class
    friend double combinedScore(const WaterSample& s);   // Unit I: friend function
};
// Unit I: FRIEND CLASS - every member function of SampleAuditor may read WaterSample's private data
class SampleAuditor {
public:
    static string rawSummary(const WaterSample& s) {
        return "audit: id=" + to_string(s.sampleId) + " source=" + to_string(s.sourceId) +
               " status=" + statusName(s.status) + (s.simulated ? " origin=simulated" : " origin=manual");
    }
};
int WaterSample::totalSamples = 0;   // static member must be defined once outside

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
    sources[sourceCount++].create(id, name, (SourceType)t, loc);
    cout << "Source registered.\n";
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
    samples[sampleCount].setBasic(id, src, date).setReadings(ph, turb, hard, simulate);
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

void menu() {
    cout << "\n========== Community Water Quality Test Manager (CA1) ==========\n"
         << " 1 Register water source    6 View one sample (findings)\n"
         << " 2 List sources             7 Summary report\n"
         << " 3 New sample (manual)      8 Edit example limits\n"
         << " 4 New sample (simulated)   9 Location tree (recursion)\n"
         << " 5 View all samples        10 Exit\n";
         
}

int main(int argc, char* argv[]) {
    unsigned seed = (unsigned)time(nullptr);
    if (argc == 3 && string(argv[1]) == "--seed") seed = (unsigned)atoi(argv[2]);
    srand(seed);
    cout << "Educational simulation. Example limits only - NOT official safety certification.\n";
    int choice;
    do {
        menu();
        choice = readInt("Choose: ", 1, 10);
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
            case 10: cout << "\nThank you for using the system. Goodbye!\n"; break;
        }
    } while (choice != 10);
    return 0;
}
