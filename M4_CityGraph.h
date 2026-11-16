// M4_CityGraph.h
// Member 4 - City Zone Map, Delivery Route and Simulated Clock
// Data Structures : Graph Adjacency Matrix + Singly Linked List (waypoints)
//                   + Edge List + Disjoint Set (used by Kruskal)
// Feature F5 : Multi Stop Route      Feature F8 : Simulated Clock
// Feature F9 : Minimum Spanning Tree route and bound (Prim and Kruskal)
// Feature S1 : Three Tier Zero Waste Pipeline
// File Handling : clock.txt , so a reload restores the moment it was saved
#ifndef M4_CITYGRAPH_H
#define M4_CITYGRAPH_H
#include <iostream>
#include <iomanip>
#include <cstring>
#include <cstdlib>
#include <cstdio>      // FILE, fopen, fprintf, fgets - C file handling
#include "M2_FoodInventory.h"     // the pipeline reads the food list
using namespace std;
#ifndef TOTAL_ZONES
#define TOTAL_ZONES 6
#endif
#define VAN_SPEED 4     // km the van covers in one hour in city traffic

// ============== PART A : C DATA STRUCTURE PART ==============
// 1) GRAPH as an adjacency matrix, dist[a][b] = km between zone a and b
struct CityGraph
{
    int dist[TOTAL_ZONES][TOTAL_ZONES];
    char zoneName[TOTAL_ZONES][25];
};

void initGraph(struct CityGraph *g)
{
    strcpy(g->zoneName[0], "Clement Town");
    strcpy(g->zoneName[1], "Rajpur Road");
    strcpy(g->zoneName[2], "Prem Nagar");
    strcpy(g->zoneName[3], "ISBT");
    strcpy(g->zoneName[4], "Sahastradhara");
    strcpy(g->zoneName[5], "Ballupur");
    int table[6][6] =
    {
        {  0,  4,  7, 10,  6, 12 },
        {  4,  0,  5,  8,  9, 14 },
        {  7,  5,  0,  6, 11, 10 },
        { 10,  8,  6,  0,  7,  9 },
        {  6,  9, 11,  7,  0,  8 },
        { 12, 14, 10,  9,  8,  0 }
    };
    for (int i = 0; i < TOTAL_ZONES; i++)
        for (int j = 0; j < TOTAL_ZONES; j++)
            g->dist[i][j] = table[i][j];
}

// O(1), one index, no searching
int getDistance(struct CityGraph *g, int from, int to)
{
    if (from < 0 || from >= TOTAL_ZONES || to < 0 || to >= TOTAL_ZONES) return -1;
    return g->dist[from][to];
}

char* getZoneName(struct CityGraph *g, int zone) { return g->zoneName[zone]; }

int getTravelTime(struct CityGraph *g, int from, int to)
{
    int km = getDistance(g, from, to);
    if (km <= 0) return 0;
    // round up, so a 14 km trip counts as 4 hours and not 3.
    // under-estimating travel would let food be sent that spoils on the way.
    int hours = (km + VAN_SPEED - 1) / VAN_SPEED;
    if (hours < 1) hours = 1;       // any trip takes at least one hour
    return hours;
}

int findNearestZone(struct CityGraph *g, int from)
{
    int best = -1, bestDist = 9999;
    for (int j = 0; j < TOTAL_ZONES; j++)
    {
        if (j == from) continue;
        if (g->dist[from][j] < bestDist) { bestDist = g->dist[from][j];  best = j; }
    }
    return best;
}

void displayZones(struct CityGraph *g)
{
    cout << "\n  ---- CITY ZONES ----" << endl;
    for (int i = 0; i < TOTAL_ZONES; i++)
        cout << "  Zone " << i << " : " << g->zoneName[i] << endl;
}

void displayMatrix(struct CityGraph *g)
{
    cout << "\n  ---- DISTANCE MATRIX (km) ----" << endl;
    cout << "        ";
    for (int j = 0; j < TOTAL_ZONES; j++) cout << setw(4) << j;
    cout << endl;
    for (int i = 0; i < TOTAL_ZONES; i++)
    {
        cout << "  Z" << i << "   ";
        for (int j = 0; j < TOTAL_ZONES; j++) cout << setw(4) << g->dist[i][j];
        cout << "   " << g->zoneName[i] << endl;
    }
}

// 2) SINGLY LINKED LIST - the driver's route, one node is one stop
struct Waypoint
{
    int zone;
    int distFromPrev;
    struct Waypoint *next;
};

struct Route
{
    struct Waypoint *head;
    int stops;
    int totalKm;
};

void initRoute(struct Route *r) { r->head = NULL;  r->stops = 0;  r->totalKm = 0; }

void addWaypoint(struct Route *r, int zone, int dist)
{
    struct Waypoint *w = (struct Waypoint *) malloc(sizeof(struct Waypoint));
    w->zone = zone;
    w->distFromPrev = dist;
    w->next = NULL;
    if (r->head == NULL)
    {
        r->head = w;
    } else
    {
        struct Waypoint *p = r->head;
        while (p->next != NULL) p = p->next;
        p->next = w;
    }
    r->stops++;
    r->totalKm = r->totalKm + dist;
}

void freeRoute(struct Route *r)
{
    struct Waypoint *p = r->head;
    while (p != NULL)
    {
        struct Waypoint *temp = p;
        p = p->next;
        free(temp);
    }
    r->head = NULL;  r->stops = 0;  r->totalKm = 0;
}

// Feature F5 : greedy nearest neighbour.
// from where we are, go to the closest zone not visited yet, repeat.
void buildRoute(struct CityGraph *g, struct Route *route, int start,
                int need[], int totalNeed)
{
    int visited[TOTAL_ZONES];
    for (int i = 0; i < TOTAL_ZONES; i++) visited[i] = 0;
    int current = start;
    visited[start] = 1;
    addWaypoint(route, start, 0);
    for (int step = 0; step < totalNeed; step++)
    {
        int best = -1, bestDist = 9999;
        for (int k = 0; k < totalNeed; k++)
        {
            int z = need[k];
            if (visited[z] == 1) continue;
            if (g->dist[current][z] < bestDist)
            {
                bestDist = g->dist[current][z];
                best = z;
            }
        }
        if (best == -1) break;              // everything visited
        addWaypoint(route, best, bestDist);
        visited[best] = 1;
        current = best;
    }
}

void displayRoute(struct CityGraph *g, struct Route *r)
{
    cout << "\n  ---- DELIVERY ROUTE (" << r->stops << " stops) ----" << endl;
    if (r->head == NULL) { cout << "  no route" << endl;  return; }
    struct Waypoint *p = r->head;
    int n = 1;
    while (p != NULL)
    {
        cout << "  " << n << ". " << g->zoneName[p->zone];
        if (p->distFromPrev > 0) cout << "   (+" << p->distFromPrev << " km)";
        else cout << "   (start)";
        cout << endl;
        p = p->next;
        n++;
    }
    cout << "  Total distance : " << r->totalKm << " km" << endl;
}

// 3) EDGE LIST - every leg between the zones on this trip, kept sorted
//    by distance so Kruskal can just walk down it from the top
struct Edge
{
    int from;
    int to;
    int km;
};

struct EdgeList
{
    struct Edge items[TOTAL_ZONES * TOTAL_ZONES];
    int count;
};

void initEdgeList(struct EdgeList *e) { e->count = 0; }

// insertion sort step, shift the longer legs right and drop the new one in
void addEdgeSorted(struct EdgeList *e, int a, int b, int km)
{
    if (e->count == TOTAL_ZONES * TOTAL_ZONES) return;
    int i = e->count - 1;
    while (i >= 0 && e->items[i].km > km)
    {
        e->items[i + 1] = e->items[i];
        i--;
    }
    e->items[i + 1].from = a;
    e->items[i + 1].to = b;
    e->items[i + 1].km = km;
    e->count++;
}

// each pair of zones once, so 4 zones give 6 legs and not 12
void buildEdgeList(struct CityGraph *g, struct EdgeList *e, int zones[], int n)
{
    initEdgeList(e);
    for (int i = 0; i < n; i++)
        for (int j = i + 1; j < n; j++)
            addEdgeSorted(e, zones[i], zones[j], g->dist[zones[i]][zones[j]]);
}

// 4) DISJOINT SET - says whether two zones are already joined up.
//    Kruskal needs it to spot a leg that would close a cycle.
struct DisjointSet
{
    int parent[TOTAL_ZONES];
};

void initSets(struct DisjointSet *s)
{
    for (int i = 0; i < TOTAL_ZONES; i++) s->parent[i] = i;     // each zone on its own
}

// climb to the top of the group
int findSet(struct DisjointSet *s, int x)
{
    while (s->parent[x] != x) x = s->parent[x];
    return x;
}

// hang one group under the other
void joinSets(struct DisjointSet *s, int a, int b)
{
    int ra = findSet(s, a);
    int rb = findSet(s, b);
    if (ra != rb) s->parent[ra] = rb;
}

// 5) THE SPANNING TREE. link[a][b] holds the km if that leg is in the
//    tree and 0 if it is not, so it is a small adjacency matrix again.
struct MSTree
{
    int link[TOTAL_ZONES][TOTAL_ZONES];
    int totalKm;
    int legs;
};

void initTree(struct MSTree *t)
{
    for (int i = 0; i < TOTAL_ZONES; i++)
        for (int j = 0; j < TOTAL_ZONES; j++) t->link[i][j] = 0;
    t->totalKm = 0;
    t->legs = 0;
}

void addLink(struct MSTree *t, int a, int b, int km)
{
    t->link[a][b] = km;
    t->link[b][a] = km;
    t->totalKm = t->totalKm + km;
    t->legs++;
}

// PRIM. start at the first zone and keep pulling in whichever zone is
// nearest to the tree we have so far. reads the matrix straight, O(V*V).
// key[i] is how far zone i is from the tree, from[i] is which tree zone
// it would hang off.
void primMST(struct CityGraph *g, struct MSTree *t, int zones[], int n, int show)
{
    int inTree[TOTAL_ZONES];
    int key[TOTAL_ZONES];
    int from[TOTAL_ZONES];
    initTree(t);
    for (int i = 0; i < n; i++) { inTree[i] = 0;  key[i] = 9999;  from[i] = -1; }
    key[0] = 0;                         // zones[0] is where the van already is
    for (int c = 0; c < n; c++)
    {
        int pick = -1, best = 9999;
        for (int i = 0; i < n; i++)
            if (inTree[i] == 0 && key[i] < best) { best = key[i];  pick = i; }
        if (pick == -1) break;
        inTree[pick] = 1;
        if (from[pick] >= 0)
        {
            addLink(t, zones[pick], zones[from[pick]], key[pick]);
            if (show == 1)
                cout << "    joined   " << g->zoneName[zones[from[pick]]] << "  -  "
                     << g->zoneName[zones[pick]] << "  (" << key[pick] << " km)" << endl;
        }
        else if (show == 1)
            cout << "    start at " << g->zoneName[zones[pick]] << endl;
        // the tree grew, so some zones may be closer to it now
        for (int v = 0; v < n; v++)
        {
            int km = g->dist[zones[pick]][zones[v]];
            if (inTree[v] == 0 && km < key[v]) { key[v] = km;  from[v] = pick; }
        }
    }
}

// KRUSKAL. walk down the sorted leg list and take a leg only when its
// two ends sit in different groups, otherwise it would close a cycle.
// a tree over n zones has exactly n-1 legs, so we can stop there.
void kruskalMST(struct CityGraph *g, struct MSTree *t, int zones[], int n, int show)
{
    struct EdgeList e;
    struct DisjointSet s;
    buildEdgeList(g, &e, zones, n);
    initSets(&s);
    initTree(t);
    for (int i = 0; i < e.count; i++)
    {
        int a = e.items[i].from;
        int b = e.items[i].to;
        if (findSet(&s, a) == findSet(&s, b))
        {
            if (show == 1)
                cout << "    skipped  " << g->zoneName[a] << "  -  " << g->zoneName[b]
                     << "  (" << e.items[i].km << " km) , would close a cycle" << endl;
            continue;
        }
        joinSets(&s, a, b);
        addLink(t, a, b, e.items[i].km);
        if (show == 1)
            cout << "    took     " << g->zoneName[a] << "  -  " << g->zoneName[b]
                 << "  (" << e.items[i].km << " km)" << endl;
        if (t->legs == n - 1) break;
    }
}

void displayTree(struct CityGraph *g, struct MSTree *t)
{
    for (int i = 0; i < TOTAL_ZONES; i++)
        for (int j = i + 1; j < TOTAL_ZONES; j++)
            if (t->link[i][j] > 0)
                cout << "      " << g->zoneName[i] << "  -  " << g->zoneName[j]
                     << "   " << t->link[i][j] << " km" << endl;
    cout << "    tree total : " << t->totalKm << " km over " << t->legs << " legs" << endl;
}

// walk the tree from the zone the van is in and write down every zone
// the first time we reach it. that order visits every stop, so it is a
// route we are allowed to drive.
void walkTree(struct MSTree *t, int at, int seen[], int order[], int *count)
{
    seen[at] = 1;
    order[*count] = at;
    (*count)++;
    for (int v = 0; v < TOTAL_ZONES; v++)
        if (t->link[at][v] > 0 && seen[v] == 0) walkTree(t, v, seen, order, count);
}

// the walk order is only a list of zones, the real km come off the road
// matrix and not off the tree
int routeLength(struct CityGraph *g, int order[], int count)
{
    int tot = 0;
    for (int i = 1; i < count; i++) tot = tot + g->dist[order[i - 1]][order[i]];
    return tot;
}

// 6) ZERO WASTE PIPELINE (Feature S1). nothing is thrown away. food
//    people can still eat stays for the ngos, food just past its time
//    goes to animal rescue, and anything older goes to compost. the
//    facilities sit in real zones, so the city graph gives the distance.
#define ZW_PEOPLE   1
#define ZW_ANIMAL   2
#define ZW_COMPOST  3
#define ANIMAL_GRACE 12     // hours past expiry that animals can still be fed

struct WasteFacility
{
    char name[30];
    int zone;
    int items;
    int meals;
};

void initFacility(struct WasteFacility *f, const char *n, int z)
{
    strcpy(f->name, n);
    f->zone = z;
    f->items = 0;
    f->meals = 0;
}

void addToFacility(struct WasteFacility *f, int meals)
{
    f->items++;
    f->meals = f->meals + meals;
}

// the whole rule in one place. lifeLeft is hours, negative means the
// food is already past its time.
int decideTier(float lifeLeft)
{
    if (lifeLeft > 0) return ZW_PEOPLE;
    if (lifeLeft >= -ANIMAL_GRACE) return ZW_ANIMAL;
    return ZW_COMPOST;
}

// pads a name out so the totals line up. setw with left would stay
// switched on and upset the distance matrix printing later.
void padName(const char *n)
{
    cout << n;
    for (int i = (int) strlen(n); i < 21; i++) cout << " ";
}

const char* tierName(int tier)
{
    if (tier == ZW_PEOPLE) return "PEOPLE";
    if (tier == ZW_ANIMAL) return "ANIMAL RESCUE";
    return "COMPOST";
}

// ================= PART B : C++ OOP PART =================
// Feature F8 : simulated clock that can be moved forward
class SimulatedClock
{
private:
    int day;
    int hour;
public:
    SimulatedClock() { day = 1;  hour = 8; }
    SimulatedClock(int d, int h) { day = d;  hour = h; }
    int getDay() { return day; }
    int getHour() { return hour; }
    // counted from the start, so food age never goes negative after midnight
    int getTotalHours() { return (day - 1) * 24 + hour; }
    // while, not if, so a jump bigger than 24 hours still works
    void advance(int hours)
    {
        hour = hour + hours;
        while (hour >= 24) { hour = hour - 24;  day++; }
    }
    void addOneHour() { advance(1); }
    void addThreeHours() { advance(3); }
    void reset() { day = 1;  hour = 8; }

    // the clock is part of the saved snapshot, see saveClock below
    void saveTo(FILE *f) { fprintf(f, "%d|%d\n", day, hour); }
    void setTime(int d, int h) { day = d;  hour = h; }

    // OPERATOR OVERLOADING
    // clk += 3  instead of  clk.advance(3)
    SimulatedClock& operator += (int hours)
    {
        advance(hours);
        return *this;
    }

    // gives the hours between two clock readings, so  now - earlier
    // works like normal subtraction
    int operator - (SimulatedClock &other)
    {
        return getTotalHours() - other.getTotalHours();
    }

    const char* getPartOfDay()
    {
        if (hour >= 5 && hour <= 11) return "Morning";
        if (hour >= 12 && hour <= 16) return "Afternoon";
        if (hour >= 17 && hour <= 20) return "Evening";
        return "Night";
    }
    void display()
    {
        cout << "Day " << day << " , ";
        if (hour < 10) cout << "0";
        cout << hour << ":00 (" << getPartOfDay() << ")";
    }
};

// FILE HANDLING for the clock. the clock has to be part of the saved
// snapshot. food.txt holds how old each item was, not the hour it was
// made, so if the clock is not put back as well then loading after the
// clock has moved forward quietly makes every item younger.
#define CLOCK_FILE "clock.txt"

int saveClock(SimulatedClock *clk)
{
    FILE *f = fopen(CLOCK_FILE, "w");
    if (f == NULL)
    {
        cout << "  Could not open " << CLOCK_FILE << " for writing." << endl;
        return -1;
    }
    clk->saveTo(f);
    fclose(f);
    cout << "  Clock    : saved as Day " << clk->getDay() << " , ";
    if (clk->getHour() < 10) cout << "0";
    cout << clk->getHour() << ":00" << endl;
    return 1;
}

int loadClock(SimulatedClock *clk)
{
    FILE *f = fopen(CLOCK_FILE, "r");
    if (f == NULL)
    {
        cout << "  Clock    : no " << CLOCK_FILE << " yet, clock left where it is." << endl;
        return -1;
    }
    char line[50];
    if (fgets(line, 50, f) == NULL)
    {
        fclose(f);
        cout << "  Clock    : file was empty, clock left where it is." << endl;
        return -1;
    }
    fclose(f);
    char *d = strtok(line, "|");
    char *h = strtok(NULL, "|");
    if (d == NULL || h == NULL)
    {
        cout << "  Clock    : bad line, clock left where it is." << endl;
        return -1;
    }
    int day = atoi(d);
    int hour = atoi(h);
    // a bad file must not be able to put the clock somewhere impossible
    if (day < 1 || hour < 0 || hour > 23)
    {
        cout << "  Clock    : values out of range, clock left where it is." << endl;
        return -1;
    }
    clk->setTime(day, hour);
    cout << "  Clock    : put back to Day " << day << " , ";
    if (hour < 10) cout << "0";
    cout << hour << ":00" << endl;
    return 1;
}

// owns one delivery route. the constructor sets it up and the
// destructor frees the waypoints, so nobody has to remember to
// call freeRoute by hand.
class DeliveryRoute
{
private:
    struct Route route;             // the C structure lives inside the class
    struct CityGraph *city;
public:
    DeliveryRoute(struct CityGraph *g)
    {
        city = g;
        initRoute(&route);
    }

    ~DeliveryRoute()
    {
        freeRoute(&route);
    }

    void build(int start, int need[], int howMany)
    {
        buildRoute(city, &route, start, need, howMany);
    }

    int getStops() { return route.stops; }
    int getTotalKm() { return route.totalKm; }
    void display() { displayRoute(city, &route); }
};


// Feature S1 : each destination decides for itself what it will take.
// adding a fourth tier later means writing one more class, the pipeline
// loop below does not change. accepts() is pure virtual so a destination
// cannot forget to answer.
class WasteDestination
{
protected:
    struct WasteFacility place;     // the C structure lives inside the class
public:
    WasteDestination(const char *n, int z) { initFacility(&place, n, z); }
    virtual ~WasteDestination() { }

    virtual int accepts(float lifeLeft) = 0;
    virtual const char* why() = 0;

    char* getName() { return place.name; }
    int getZone() { return place.zone; }
    int getItems() { return place.items; }
    int getMeals() { return place.meals; }
    void take(int meals) { addToFacility(&place, meals); }
};

class AnimalRescue : public WasteDestination
{
public:
    AnimalRescue(int z) : WasteDestination("Animal Rescue Centre", z) { }
    int accepts(float lifeLeft) { return (decideTier(lifeLeft) == ZW_ANIMAL) ? 1 : 0; }
    const char* why() { return "past its time for people, still safe for animals"; }
};

class CompostSite : public WasteDestination
{
public:
    CompostSite(int z) : WasteDestination("Compost Yard", z) { }
    int accepts(float lifeLeft) { return (decideTier(lifeLeft) == ZW_COMPOST) ? 1 : 0; }
    const char* why() { return "too old for anything to eat, goes back to the soil"; }
};

// walks the inventory, decides where every item belongs, and only then
// lets M2 clear the expired ones out. returns how many were cleared.
int runZeroWaste(struct ActiveFoodList *food, struct CityGraph *city, int currentHour)
{
    AnimalRescue animals(3);        // the rescue centre is at ISBT
    CompostSite compost(5);         // the compost yard is out at Ballupur
    WasteDestination *where[2];
    where[0] = &animals;
    where[1] = &compost;

    cout << "\n  --- ZERO WASTE PIPELINE (Feature S1) ---" << endl;
    if (food->count == 0) { cout << "  the inventory is empty" << endl;  return 0; }

    int peopleItems = 0, peopleMeals = 0, unplaced = 0;
    struct FoodNode *p = food->head;
    while (p != NULL)
    {
        float lifeLeft = p->food->getRemainingLife(currentHour);
        if (decideTier(lifeLeft) == ZW_PEOPLE)
        {
            peopleItems++;
            peopleMeals = peopleMeals + p->food->getQuantity();
            p = p->next;
            continue;
        }
        // ask each destination in turn, the first one that takes it wins
        int placed = 0;
        for (int i = 0; i < 2 && placed == 0; i++)
        {
            if (where[i]->accepts(lifeLeft) == 1)       // virtual call
            {
                int km = getDistance(city, p->food->getZone(), where[i]->getZone());
                cout << "  " << p->food->getId() << " " << p->food->getName()
                     << " : expired " << (-lifeLeft) << " hrs ago  ->  "
                     << where[i]->getName() << " at "
                     << getZoneName(city, where[i]->getZone())
                     << " , " << km << " km" << endl;
                where[i]->take(p->food->getQuantity());
                placed = 1;
            }
        }
        if (placed == 0) unplaced++;
        p = p->next;
    }

    // M2 owns the list, so M2 does the deleting. this file only decides
    // where each item should go.
    int cleared = removeExpired(food, currentHour);

    cout << "\n  ---- WHERE IT ALL WENT ----" << endl;
    cout << "  ";  padName("People");  cout << ": " << peopleItems << " item(s) , "
         << peopleMeals << " meals , still in the inventory" << endl;
    for (int i = 0; i < 2; i++)
        { cout << "  ";  padName(where[i]->getName());  cout << ": " << where[i]->getItems()
             << " item(s) , " << where[i]->getMeals() << " meals" << endl; }
    cout << "  " << cleared << " item(s) cleared from the active inventory." << endl;
    if (unplaced > 0)
        cout << "  " << unplaced << " item(s) fitted no tier, that should not happen." << endl;
    else
        cout << "  Nothing was thrown away." << endl;
    return cleared;
}

// ================= MENU FUNCTIONS =================
void distanceMenu(struct CityGraph *g)
{
    int a, b;
    cout << "\n  --- CHECK DISTANCE ---" << endl;
    displayZones(g);
    cout << "\n  From zone (0-5) : ";  cin >> a;
    cout << "  To zone   (0-5) : ";    cin >> b;
    int km = getDistance(g, a, b);
    if (km < 0) { cout << "  Wrong zone number." << endl;  return; }
    cout << "\n  " << g->zoneName[a] << "  ->  " << g->zoneName[b] << endl;
    cout << "  Distance    : " << km << " km" << endl;
    cout << "  Travel time : " << getTravelTime(g, a, b) << " hrs" << endl;
    int near = findNearestZone(g, a);
    cout << "  Nearest zone to " << g->zoneName[a] << " is " << g->zoneName[near]
         << " (" << g->dist[a][near] << " km)" << endl;
}

// walk the tree out from the start zone and measure what that order costs.
int walkMST(struct CityGraph *g, struct MSTree *t, int start, int order[], int *count)
{
    int seen[TOTAL_ZONES];
    for (int i = 0; i < TOTAL_ZONES; i++) seen[i] = 0;
    *count = 0;
    walkTree(t, start, seen, order, count);
    return routeLength(g, order, *count);
}

// print a visiting order as a numbered list, same layout displayRoute
// uses, so the MST walk and the greedy route look the same on screen.
void displayOrder(struct CityGraph *g, int order[], int count)
{
    cout << "\n  ---- DELIVERY ROUTE (" << count << " stops) ----" << endl;
    int total = 0;
    for (int i = 0; i < count; i++)
    {
        cout << "  " << (i + 1) << ". " << g->zoneName[order[i]];
        if (i == 0) cout << "   (start)";
        else
        {
            int leg = g->dist[order[i - 1]][order[i]];
            total = total + leg;
            cout << "   (+" << leg << " km)";
        }
        cout << endl;
    }
    cout << "  Total distance : " << total << " km" << endl;
}

void routeMenu(struct CityGraph *g)
{
    int start, howMany;
    int typed[TOTAL_ZONES];
    cout << "\n  --- BUILD DELIVERY ROUTE ---" << endl;
    displayZones(g);
    cout << "\n  Van starts at zone (0-5) : ";  cin >> start;
    cout << "  How many stops (1-5)     : ";    cin >> howMany;
    if (start < 0 || start >= TOTAL_ZONES || howMany < 1 || howMany > 5)
    {
        cout << "  Wrong input." << endl;
        return;
    }
    for (int i = 0; i < howMany; i++)
    {
        cout << "  Stop " << (i + 1) << " zone : ";
        cin >> typed[i];
        if (typed[i] < 0 || typed[i] >= TOTAL_ZONES)
        {
            cout << "  Wrong zone number." << endl;
            return;
        }
    }

    // throw away a zone typed twice, and the zone the van is already
    // standing in. a repeat would be counted as an extra stop and the
    // spanning tree leg count would come out wrong.
    int need[TOTAL_ZONES];
    int stops = 0, dropped = 0;
    for (int i = 0; i < howMany; i++)
    {
        int already = 0;
        if (typed[i] == start) already = 1;
        for (int j = 0; j < stops; j++) if (need[j] == typed[i]) already = 1;
        if (already == 1) { dropped++;  continue; }
        need[stops] = typed[i];
        stops++;
    }
    if (dropped > 0) cout << "\n  " << dropped << " repeated zone(s) ignored." << endl;
    if (stops == 0) { cout << "  Nothing left to visit." << endl;  return; }

    DeliveryRoute route(g);
    route.build(start, need, stops);    // destructor frees the waypoints on its own

    // the zone the van starts in has to go into the tree too, or the
    // bound would quietly ignore the first leg of the trip
    int zones[TOTAL_ZONES];
    zones[0] = start;
    for (int i = 0; i < stops; i++) zones[i + 1] = need[i];
    int n = stops + 1;

    // both trees still get built and both walks still get measured.
    // only the running commentary is left out, the last flag turns it off.
    struct MSTree tree[2];
    int bestWalk = 9999, bestCount = 0;
    int bestOrder[TOTAL_ZONES];
    for (int i = 0; i < TOTAL_ZONES; i++) bestOrder[i] = start;

    for (int p = 0; p < 2; p++)
    {
        int order[TOTAL_ZONES], count = 0;
        initTree(&tree[p]);
        if (p == 0) primMST(g, &tree[p], zones, n, 0);
        else        kruskalMST(g, &tree[p], zones, n, 0);
        int km = walkMST(g, &tree[p], start, order, &count);
        if (km < bestWalk)
        {
            bestWalk = km;
            bestCount = count;
            for (int i = 0; i < count; i++) bestOrder[i] = order[i];
        }
    }

    // the greedy route and the two tree walks are all guesses. show
    // whichever came out shortest, that can never be worse than either.
    if (bestWalk < route.getTotalKm()) displayOrder(g, bestOrder, bestCount);
    else                               route.display();
}

void clockMenu(SimulatedClock *clk)
{
    int choice;
    SimulatedClock before = *clk;       // keep a copy to subtract from later
    cout << "\n  --- SIMULATED CLOCK ---" << endl;
    cout << "  Now : ";  clk->display();  cout << endl;
    cout << "\n  1. Forward 1 hour" << endl;
    cout << "  2. Forward 3 hours" << endl;
    cout << "  3. Forward custom hours" << endl;
    cout << "  4. Reset to Day 1, 08:00" << endl;
    cout << "  Choice : ";
    cin >> choice;
    // *clk += n  uses our overloaded += operator
    if (choice == 1) *clk += 1;
    else if (choice == 2) *clk += 3;
    else if (choice == 3) { int h;  cout << "  Hours : ";  cin >> h;  *clk += h; }
    else if (choice == 4) clk->reset();
    else { cout << "  Wrong choice." << endl;  return; }
    cout << "\n  New time : ";  clk->display();  cout << endl;
    // *clk - before  uses our overloaded - operator
    cout << "  Moved forward by : " << (*clk - before) << " hrs" << endl;
}

#endif
