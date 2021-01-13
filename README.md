# FlowField pathfinding
 
 A flow field is a way of pathfinding that works on a grid, for every cell in the grid a vector is calculated, any agent traversing the grid can then use the vector assigned to the cell the agent is currently in to take the shortest route to the destination.
 
 ![Flowfield](https://github.com/MauroVanHoutte/FlowField/blob/main/Gifs/Flowfield.gif)
 
# Design

The flow field is designed to work on the grid and graph implementations of the Elite framework, the box2d library is used as physics engine and the interface is made using ImGui.
The algorithm itself runs in 2 steps, the first step is calculating the cost from the destination cell to each other cell in the grid, the second step is creating a unit vector for each cell pointing towards the cells neighbour with the cheapest cost.
To let the agents utilize the flow field, the cell the agent is in is calculated from then align the agents velocity with the vector that corresponds to his current cell

# Implementation

Calculation of each cells cost in pseudo code:

```
vector<Node> openlist // vector that keeps track of nodes that have to be calculated
list<Node> closedlist // list that keeps track of nodes that have been calculated

openlist.push_back(destinationNode) // start from the destination and expands outwards
closedlist.push_back(destinationNode)

while (!openlist.empty)
   node = FindLowestCostNode(openlist) // lowest cost always has to be calculated first, especially important when connection costs are variable
   openlist.remove(node)
   cellcost[node.Index] = node.cost // cellcost is a vector that is a member of the flowfield class 
 
   for (connection : node.connections)
     newNode = connection.GetTo // get the connected node
     newNode.cost = node.cost + connection.cost
  
     if (!find(closedlist, newNode)) //nodes that have been calculated already wont get added again
       closedlist.push_back(newNode)
       openlist.push_back(newNode)
```
Calculation of each cells vector:

```
for (node : graph.GetAllNodes)
  cheapestNode = FindCheapestNeighbour(node)
  flowfield[node.Index] = Normalized(cheapestNode.worldPos - node.worldPos) // flowfield contains the unit vectors for each cell

flowfield[endNode.Index] = ZeroVector
```

The brown cells here have a higher cost

![Brown tiles have a higher cost](https://github.com/MauroVanHoutte/FlowField/blob/main/Gifs/VariableCellCosts.gif)


While implementing the flowfield i had 2 other systems in mind that use the flowfield as a foundation.

1 : Teleporter

![Teleporters](https://github.com/MauroVanHoutte/FlowField/blob/main/Gifs/Teleporters.gif)

  The implementation exists of a struct holding the node indices where the 2 teleporters are located and a variable that keeps track of which teleporter is closer to the destination.
  To adjust the flowfield to this, during the calculation of the cell costs and before adding a node's connections to the openlist, if the node is one of the teleporters add the
  other the other teleporter to the openlist with the same cost.
  In Pseudo code that looks like this:
  
```
...
node = FindLowestCostNode(openlist)
   openlist.remove(node)
   cellcost[node.Index] = node.cost 
 
   if (node.Index == teleporter)
     teleporterNode = teleporter.other // other teleporter connecting to this one
     openlist.pushback(teleporterNode)
     closedlist.pushback(teleporterNode)
   
   for (connection : node.connections)
     newNode = connection.GetTo
     newNode.cost = node.cost + connection.cost
...
```


2 : Traffic

  The problem
  
![No Measure Against Traffic](https://github.com/MauroVanHoutte/FlowField/blob/main/Gifs/Clutter.gif)

  The solution
  
![Anti Traffic](https://github.com/MauroVanHoutte/FlowField/blob/main/Gifs/AntiTraffic.gif)

  To combat the cluttering when the vectors of each cell are calculated the cell cost is increased for every agent currently in the cell,
  this implementation does have an impact on performance as the vectors of the cell now need to be calculated every frame
  
 # Future work
 
 Adapting to 3d could have some interesting use cases
  

    
