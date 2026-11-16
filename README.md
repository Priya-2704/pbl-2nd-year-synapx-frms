# Food Redistribution Management System (FRMS)

A C++ console project for Data Structures in C and OOP with C++.
## What the project is about

Restaurants, hostel messes and caterers throw away a lot of food that is
still good to eat. At the same time shelters and NGOs nearby do not have
enough. The problem is that cooked food does not stay safe for long, so
the food has to reach someone quickly or it is wasted anyway.

This program connects the two sides. Donors add their surplus food, NGOs
ask for meals, and the program decides who should get what.

It does not just serve requests in the order they arrive. Serving in order
is unfair, because an emergency can end up waiting behind a routine request
that came in earlier. Instead every possible food-and-NGO pair gets a score
out of 100, based on three things:

- how urgent the NGO is
- how long it has already been waiting
- how close the food is

The meals are then handed out in score order. One large donation can be
split between several shelters, and a shelter that is already full is
passed over even if its score was high.

Before scoring, the program also checks something important. Food that is
fine right now may not be fine after the van has driven across the city.
So the travel time is subtracted from the remaining shelf life, and anything
that would spoil on the way is rejected instead of delivered.

Food that is past its time is not thrown away. If it is only a few hours
over it goes to an animal rescue centre, and if it is older than that it
goes to a compost yard.

There is also a fake clock built in. You can jump the time forward by a few
hours and watch the food actually go bad, instead of waiting for real hours
to pass.

## How to run

```
g++ -std=c++11 main.cpp -o frms.exe
./frms.exe
```

Press 22 to load the sample data, then 15 to run the matching.

## Files

```
M1_UserDirectory.h    hash table, donor trust rating, surplus forecast
M2_FoodInventory.h    doubly linked list, food decay, shelf life, storage
M3_MatchingEngine.h   circular queue, max heap, priority score, splitting
M4_CityGraph.h        adjacency matrix, delivery route, clock, spanning tree,
                      zero waste pipeline
main.cpp              menu
```

## What is working

- Donors and NGOs can be registered, searched and deleted
- Each donor gets a trust score, and reliable donors are approved automatically
- The program forecasts how much surplus a donor will have in the evening
- Food can be added, listed and routed away once it expires
- Cooked food spoils faster than packed food, and open food fastest of all
- Food kept in a cold room spoils slower than food left out
- Food is sorted into four hygiene levels based on how much life is left
- Travel time is subtracted from shelf life, so unsafe deliveries are refused
- Every food and NGO pair is scored and ranked, using a max heap
- The meals are actually handed out, and one donation can be split between
  several shelters
- Expired food is sent to animal rescue, and then to compost
- A delivery route is built that visits the nearest stop each time
- A minimum spanning tree is built with both Prim and Kruskal, which gives a
  second route and a lower bound that shows how close our route is to the best
- The clock can be moved forward to make food spoil during a demo
- Everything is saved to a file, so the data is still there after closing
- Everything runs from one menu


