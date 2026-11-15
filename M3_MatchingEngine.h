// M3_MatchingEngine.h
// Member 3 - Priority Matching Engine
// Data Structures : Circular Queue (ngo requests) + Max Heap (priority list)
// File Handling   : requests.txt , one waiting request per line
// Feature F2 : Priority Score  P = 0.5E + 0.3W + 0.2Z
// Feature F3 : One donation can be split between several shelters
#ifndef M3_MATCHINGENGINE_H
#define M3_MATCHINGENGINE_H
#include <iostream>
#include <iomanip>
#include <cstring>
#include <cstdlib>
#include <cstdio>      // FILE, fopen, fprintf, fgets - C file handling
#include "M2_FoodInventory.h"   // Feature F3 reads the food quantities
using namespace std;
#define MAX_REQUESTS 5
#define MAX_MATCHES 50
#ifndef TOTAL_ZONES
#define TOTAL_ZONES 6
#endif

// ================= PART B : C++ OOP PART =================
class Request
{
private:
    char reqId[20];
    char ngoId[20];
    int mealsNeeded;
    int urgency;
    int arrivalHour;
    int zone;
public:
    Request(const char *rid, const char *nid, int meals, int urg, int arrival, int z)
    {
        strcpy(reqId, rid);  strcpy(ngoId, nid);
        mealsNeeded = meals;  urgency = urg;  arrivalHour = arrival;  zone = z;
    }
    char* getReqId() { return reqId; }
    char* getNgoId() { return ngoId; }
    int getMeals() { return mealsNeeded; }
    int getUrgency() { return urgency; }
    int getArrivalHour() { return arrivalHour; }
    int getZone() { return zone; }
    // waited hours, not the arrival hour, because the clock restarts
    void saveTo(FILE *f, int currentHour)
    {
        fprintf(f, "%s|%s|%d|%d|%d|%d\n", reqId, ngoId, mealsNeeded,
                urgency, currentHour - arrivalHour, zone);
    }
    // E part, urgency 1-10 becomes 10-100
    float getE() { return urgency * 10; }
    // W part, every hour of waiting adds 10, stopped at 100
    float getW(int currentHour)
    {
        int waited = currentHour - arrivalHour;
        if (waited < 0) waited = 0;
        float w = waited * 10;
        if (w > 100) w = 100;
        return w;
    }
    void display(int currentHour)
    {
        cout << "  " << reqId << " | NGO " << ngoId << " | " << mealsNeeded
             << " meals | Zone " << zone << " | Urgency " << urgency
             << " | Waiting " << (currentHour - arrivalHour) << " hrs" << endl;
    }
};

// Z part, nearer zone gets a higher score
float getZoneScore(int distance)
{
    float z = 100 - (distance * 5);
    if (z < 0) z = 0;
    return z;
}

class MatchPair
{
private:
    char foodId[20];
    char ngoId[20];
    int meals;
    int distance;
    float E, W, Z;
    float priority;
    int seq;            // the order it went in, used to break ties
public:
    MatchPair(const char *fid, const char *nid, int m, int dist,
              float e, float w, float z, int s)
    {
        strcpy(foodId, fid);  strcpy(ngoId, nid);
        meals = m;  distance = dist;  E = e;  W = w;  Z = z;
        priority = 0.5 * E + 0.3 * W + 0.2 * Z;     // Feature F2
        seq = s;
    }
    char* getFoodId() { return foodId; }
    char* getNgoId() { return ngoId; }
    float getPriority() { return priority; }
    int getSeq() { return seq; }
    int getMeals() { return meals; }
    int getDistance() { return distance; }
    // compare two pairs by their priority score
    int operator > (MatchPair &other) { return (priority > other.priority) ? 1 : 0; }
    int operator < (MatchPair &other) { return (priority < other.priority) ? 1 : 0; }
    void display()
    {
        cout << "  " << foodId << " -> " << ngoId << " | " << meals
             << " meals | " << distance << " km | P = " << priority << endl;
    }
    void showFormula()
    {
        cout << "  " << foodId << " -> " << ngoId << endl;
        cout << "      P = 0.5(" << E << ") + 0.3(" << W << ") + 0.2(" << Z
             << ") = " << priority << endl;
    }
};

// ============== PART A : C DATA STRUCTURE PART ==============
// 1) CIRCULAR QUEUE - ngo requests in first come first serve order
struct RequestQueue
{
    Request *items[MAX_REQUESTS];
    int front;
    int rear;
    int count;
};

// rear starts at -1 because enqueue increases it first
void initQueue(struct RequestQueue *q)
{
    q->front = 0;  q->rear = -1;  q->count = 0;
}

int isQueueFull(struct RequestQueue *q) { return (q->count == MAX_REQUESTS) ? 1 : 0; }

int isQueueEmpty(struct RequestQueue *q) { return (q->count == 0) ? 1 : 0; }

// add at the rear, % makes it come back to 0
int enqueue(struct RequestQueue *q, Request *r)
{
    if (isQueueFull(q) == 1) { cout << "  Queue is full." << endl;  return 0; }
    q->rear = (q->rear + 1) % MAX_REQUESTS;
    q->items[q->rear] = r;
    q->count++;
    return 1;
}

Request* dequeue(struct RequestQueue *q)
{
    if (isQueueEmpty(q) == 1) return NULL;
    Request *r = q->items[q->front];
    q->front = (q->front + 1) % MAX_REQUESTS;
    q->count--;
    return r;
}

// walk count items, not front to rear, because it may have wrapped
void displayQueue(struct RequestQueue *q, int currentHour)
{
    cout << "\n  ---- REQUEST QUEUE (" << q->count << " waiting) ----" << endl;
    if (isQueueEmpty(q) == 1) { cout << "  queue is empty" << endl;  return; }
    cout << "  front index = " << q->front << " , rear index = " << q->rear << endl;
    int i = q->front;
    for (int n = 0; n < q->count; n++)
    {
        cout << "  [" << i << "] ";
        q->items[i]->display(currentHour);
        i = (i + 1) % MAX_REQUESTS;
    }
}

// 2) MAX HEAP - the priority list. the biggest score is always at the
//    root, which is index 0. the rule is only that a parent is at least
//    as good as its two children, nothing is said about left against
//    right. that is a weaker promise than a fully sorted array, and it
//    is exactly why insert and remove cost O(log n) here instead of the
//    O(n) the sorted array used to cost.
//
//    the tree is kept in a plain array. for the node at i :
//        parent   (i - 1) / 2
//        left      2i + 1
//        right     2i + 2
struct MatchHeap
{
    MatchPair *items[MAX_MATCHES];
    int size;
};

void initMatchHeap(struct MatchHeap *h) { h->size = 0; }

int parentOf(int i) { return (i - 1) / 2; }
int leftOf(int i)   { return 2 * i + 1; }
int rightOf(int i)  { return 2 * i + 2; }

// higher score wins. two pairs can score exactly the same, so the one
// that went in first wins the tie. without that the order of equal
// scores would depend on the shape of the tree and could change.
int beats(MatchPair *a, MatchPair *b)
{
    if (*a > *b) return 1;                  // uses our overloaded >
    if (*b > *a) return 0;
    return (a->getSeq() < b->getSeq()) ? 1 : 0;
}

void swapPairs(struct MatchHeap *h, int a, int b)
{
    MatchPair *t = h->items[a];
    h->items[a] = h->items[b];
    h->items[b] = t;
}

// the new item is put on the end and climbs while it beats its parent
void heapifyUp(struct MatchHeap *h, int i)
{
    while (i > 0 && beats(h->items[i], h->items[parentOf(i)]) == 1)
    {
        swapPairs(h, i, parentOf(i));
        i = parentOf(i);
    }
}

// the last item is moved to the root and sinks while a child beats it.
// it has to swap with the BETTER of the two children, otherwise the
// parent rule would still be broken after the swap.
void heapifyDown(struct MatchHeap *h, int i)
{
    while (1)
    {
        int best = i;
        int l = leftOf(i);
        int r = rightOf(i);
        if (l < h->size && beats(h->items[l], h->items[best]) == 1) best = l;
        if (r < h->size && beats(h->items[r], h->items[best]) == 1) best = r;
        if (best == i) break;               // nobody below is better
        swapPairs(h, i, best);
        i = best;
    }
}

// O(log n), the new item climbs at most the height of the tree
int insertHeap(struct MatchHeap *h, MatchPair *m)
{
    if (h->size == MAX_MATCHES) { cout << "  List is full." << endl;  return 0; }
    h->items[h->size] = m;
    h->size++;
    heapifyUp(h, h->size - 1);
    return 1;
}

// take the root, move the last item up to fill the hole, then let it
// sink. also O(log n). the sorted array had to shift every item left.
MatchPair* removeBest(struct MatchHeap *h)
{
    if (h->size == 0) return NULL;
    MatchPair *best = h->items[0];
    h->size--;
    if (h->size > 0)
    {
        h->items[0] = h->items[h->size];
        heapifyDown(h, 0);
    }
    return best;
}

void freeMatchHeap(struct MatchHeap *h)
{
    for (int i = 0; i < h->size; i++) delete h->items[i];
    h->size = 0;
}

// 3) FILE HANDLING - the waiting requests are lost when the program
//    closes. one request per line. there is no tag field here because
//    Request has no child classes, every line is the same shape.
#define REQUEST_FILE "requests.txt"

int saveRequests(struct RequestQueue *q, int currentHour)
{
    FILE *f = fopen(REQUEST_FILE, "w");
    if (f == NULL)
    {
        cout << "  Could not open " << REQUEST_FILE << " for writing." << endl;
        return -1;
    }
    // walk count items from the front, not front to rear, because the
    // circular queue may have wrapped round
    int i = q->front, written = 0;
    for (int n = 0; n < q->count; n++)
    {
        q->items[i]->saveTo(f, currentHour);
        written++;
        i = (i + 1) % MAX_REQUESTS;
    }
    fclose(f);
    cout << "  Requests : " << written << " saved to " << REQUEST_FILE << endl;
    return written;
}

int loadRequests(struct RequestQueue *q, int currentHour)
{
    FILE *f = fopen(REQUEST_FILE, "r");
    if (f == NULL)
    {
        cout << "  Requests : no " << REQUEST_FILE << " to read yet." << endl;
        return -1;
    }
    char line[200];
    int added = 0, skipped = 0;
    while (fgets(line, 200, f) != NULL)
    {
        if (line[0] == '\n') continue;              // blank line
        char *rid    = strtok(line, "|");
        char *nid    = strtok(NULL, "|");
        char *meals  = strtok(NULL, "|");
        char *urg    = strtok(NULL, "|");
        char *waited = strtok(NULL, "|");
        char *zone   = strtok(NULL, "|");
        if (rid == NULL || nid == NULL || meals == NULL || urg == NULL ||
            waited == NULL || zone == NULL)
        {
            skipped++;
            continue;
        }
        // the queue has no search function, so scan it the way
        // displayQueue does, count items from the front, wrapping round
        int already = 0;
        int k = q->front;
        for (int n = 0; n < q->count; n++)
        {
            if (strcmp(q->items[k]->getReqId(), rid) == 0) { already = 1;  break; }
            k = (k + 1) % MAX_REQUESTS;
        }
        if (already == 1) { skipped++;  continue; }

        // the arrival hour is rebuilt from how long the request had
        // already been waiting when it was saved
        Request *r = new Request(rid, nid, atoi(meals), atoi(urg),
                                 currentHour - atoi(waited), atoi(zone));
        if (enqueue(q, r) == 1) added++;
        else { delete r;  skipped++; }      // queue was full, do not leak it
    }
    fclose(f);
    cout << "  Requests : " << added << " loaded";
    if (skipped > 0) cout << " , " << skipped << " skipped (already there, queue full or bad line)";
    cout << endl;
    return added;
}

// ================= THE ENGINE CLASS =================
// joins the queue and the priority list together
class MatchingEngine
{
private:
    struct RequestQueue queue;
    struct MatchHeap matches;
    int nextSeq;                // counts candidates in, for the tie-break
public:
    MatchingEngine() { initQueue(&queue);  initMatchHeap(&matches);  nextSeq = 0; }
    ~MatchingEngine()
    {
        while (isQueueEmpty(&queue) == 0) delete dequeue(&queue);
        freeMatchHeap(&matches);
    }
    struct RequestQueue* getQueue() { return &queue; }
    struct MatchHeap* getMatches() { return &matches; }
    int getQueueCount() { return queue.count; }
    int getMatchCount() { return matches.size; }
    // returns 0 if the queue was full, so the caller can clean up
    int addRequest(Request *r) { return enqueue(&queue, r); }
    void addCandidate(const char *foodId, Request *r, int distance,
                      int mealsGiven, int currentHour)
    {
        MatchPair *m = new MatchPair(foodId, r->getNgoId(), mealsGiven, distance,
                                     r->getE(), r->getW(currentHour),
                                     getZoneScore(distance), nextSeq);
        nextSeq++;
        if (insertHeap(&matches, m) == 0)
            delete m;               // list was full, do not leak it
    }
    // Feature F3 : the heap says which pairing is worth doing first.
    // this walks them in that order and actually hands the meals out.
    // one donation can be split between several shelters, and a shelter
    // that is already full is passed over even if its score was high.
    //
    // before this, the screen listed every pairing that was possible,
    // so the same 100 meals appeared against three different ngos and
    // nothing was ever subtracted. now the meals run out.
    void runMatching(struct ActiveFoodList *list)
    {
        cout << "\n  ---- MATCHING RESULT (highest priority first) ----" << endl;
        if (matches.size == 0) { cout << "  no candidates" << endl;  return; }

        // how much is left of each food item, and how much each request
        // still needs. parallel arrays, one entry per item.
        char fid[MAX_MATCHES][20];
        int  fleft[MAX_MATCHES];
        int  fused[MAX_MATCHES];        // how many shelters it was split across
        int  fcount = 0;
        struct FoodNode *p = list->head;
        while (p != NULL && fcount < MAX_MATCHES)
        {
            strcpy(fid[fcount], p->food->getId());
            fleft[fcount] = p->food->getQuantity();
            fused[fcount] = 0;
            fcount++;
            p = p->next;
        }

        char rid[MAX_REQUESTS][20];
        int  rneed[MAX_REQUESTS];
        int  rleft[MAX_REQUESTS];
        int  rcount = 0;
        int  qi = queue.front;
        for (int n = 0; n < queue.count; n++)
        {
            strcpy(rid[rcount], queue.items[qi]->getNgoId());
            rneed[rcount] = queue.items[qi]->getMeals();
            rleft[rcount] = queue.items[qi]->getMeals();
            rcount++;
            qi = (qi + 1) % MAX_REQUESTS;
        }

        int rank = 1, handed = 0;
        while (matches.size > 0)
        {
            MatchPair *best = removeBest(&matches);
            int fi = -1, ri = -1;
            for (int k = 0; k < fcount; k++)
                if (strcmp(fid[k], best->getFoodId()) == 0) fi = k;
            for (int k = 0; k < rcount; k++)
                if (strcmp(rid[k], best->getNgoId()) == 0) ri = k;

            cout << "  " << rank << ". " << best->getFoodId() << " -> "
                 << best->getNgoId() << " | " << best->getDistance()
                 << " km | P = " << best->getPriority() << " | ";

            if (fi < 0 || ri < 0)
                cout << "skipped, the food or the request is gone" << endl;
            else if (rleft[ri] == 0)
                cout << "skipped, " << rid[ri] << " is already full" << endl;
            else if (fleft[fi] == 0)
                cout << "skipped, " << fid[fi] << " is all given away" << endl;
            else
            {
                // give whichever is smaller, what is left or what is needed
                int give = (fleft[fi] < rleft[ri]) ? fleft[fi] : rleft[ri];
                fleft[fi] = fleft[fi] - give;
                rleft[ri] = rleft[ri] - give;
                fused[fi]++;
                handed = handed + give;
                cout << "gave " << give << " meals";
                if (rleft[ri] > 0) cout << " , " << rid[ri] << " still needs " << rleft[ri];
                else cout << " , " << rid[ri] << " is now full";
                if (fleft[fi] > 0) cout << " , " << fleft[fi] << " left of " << fid[fi];
                cout << endl;
            }
            delete best;
            rank++;
        }

        // ---- what actually happened ----
        int splitCount = 0, filled = 0, needTotal = 0, shortTotal = 0;
        for (int k = 0; k < fcount; k++) if (fused[k] > 1) splitCount++;
        for (int k = 0; k < rcount; k++)
        {
            needTotal = needTotal + rneed[k];
            shortTotal = shortTotal + rleft[k];
            if (rleft[k] == 0) filled++;
        }

        cout << "\n  ---- WHAT WAS HANDED OUT (Feature F3) ----" << endl;
        cout << "  Meals asked for   : " << needTotal << endl;
        cout << "  Meals handed out  : " << handed << endl;
        cout << "  Requests filled   : " << filled << " of " << rcount << endl;
        cout << "  Donations split   : " << splitCount;
        if (splitCount > 0)
        {
            cout << "  (";
            int shown = 0;
            for (int k = 0; k < fcount; k++)
                if (fused[k] > 1)
                {
                    if (shown > 0) cout << " , ";
                    cout << fid[k] << " went to " << fused[k] << " shelters";
                    shown++;
                }
            cout << ")";
        }
        cout << endl;
        if (shortTotal > 0)
        {
            cout << "  Still short       : " << shortTotal << " meals (";
            int shown = 0;
            for (int k = 0; k < rcount; k++)
                if (rleft[k] > 0)
                {
                    if (shown > 0) cout << " , ";
                    cout << rid[k] << " needs " << rleft[k];
                    shown++;
                }
            cout << ")" << endl;
        }
        else cout << "  Every request was filled." << endl;

        int spare = 0;
        for (int k = 0; k < fcount; k++) spare = spare + fleft[k];
        if (spare > 0)
            cout << "  Meals left over   : " << spare
                 << " , still in the inventory for the next round" << endl;
    }
};

// ================= MENU FUNCTIONS =================
void addRequestMenu(MatchingEngine *engine, int currentHour)
{
    char reqId[20], ngoId[20];
    int meals, urgency, zone;
    cout << "\n  --- RAISE NGO REQUEST ---" << endl;
    cout << "  Request ID : ";      cin >> setw(20) >> reqId;
    cout << "  NGO ID     : ";      cin >> setw(20) >> ngoId;
    cout << "  Meals needed   : ";  cin >> meals;
    cout << "  Urgency (1-10) : ";  cin >> urgency;
    cout << "  Zone (0-5)     : ";  cin >> zone;
    if (meals <= 0 || urgency < 1 || urgency > 10 || zone < 0 || zone >= TOTAL_ZONES)
    {
        cout << "  Wrong value entered, request not added." << endl;
        return;
    }
    Request *r = new Request(reqId, ngoId, meals, urgency, currentHour, zone);
    if (engine->addRequest(r) == 1)
        cout << "\n  Request added to the queue." << endl;
    else
        delete r;                   // queue was full, do not leak it
}

void serveNextMenu(MatchingEngine *engine, int currentHour)
{
    cout << "\n  --- SERVE NEXT REQUEST (FIFO) ---" << endl;
    Request *r = dequeue(engine->getQueue());
    if (r == NULL) { cout << "  Queue is empty." << endl;  return; }
    cout << "  Serving : ";
    r->display(currentHour);
    delete r;
}

#endif
