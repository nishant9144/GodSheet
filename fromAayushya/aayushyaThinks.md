
Order of execution
- take input for a cell
- parse it and categorise it in arithmetic value, expression or function (or range) of other cells
- in case it is among the first two, just change the value and update the affected cells
- in case it is among the last two, you need to update the dependencies set -> check for circular dependencies
- if there is a circular dependency you need to report it and stop executing
- else you need to calculate the value of the current cell and check for division by zero error while re-calculting the value
- after you have updated the value of the current cell, you should then form a graph of the affected cells(dependent cells)
- then topo sort this graph into a set of cells(nodes)
- then update the value of each cell/node based iteratively in this sorted set
- if at any point you encounter a div by zero error, you need to mark the value of each cell in the affected cells graph to ERR including the current cell as well
- if there is no division by zero error, just update the values of each affected cell

Functions required:
- parse_input() - leave this for now

- update_dependencies(curr_cell, set of new dependencies) -> updates the dependencies set according to the provided algorithm and returns nothing
    Dependency set:
    - you get a new set(D_new) of dependencies first of all for this cell
    - you also have the old set (D_old) of dependencies of the curr cell
    - you then have to iterate over the D_new, and find out if the element belongs to D_old (map or set find operation)
        - if the element belongs to it: delete it from D_old
        - else: go to the element cell and add this current cell to its dependents set
    - then iterate over the remaining D_old cell and for each element in that, you must remove the current cell from it's dependents set
    - then make curr_cell.dependencies = D_new

- check_circular_dependencies (curr_cell, dependencies) -> forms the graph and checks for cycles as mentioned below -> returns a bool (true:nocycle, false: cycle present, stop execution & report error)
    How to check for circular dependecy?
    - go into the set of its dependencies and keep going deeper and form a graph of these nodes
    - you should keep a check of the visited cells (nodes here) in order to construct a proper graph
    - then see if there exists a cycle here
    - if there exists a cycle then you should report the error and for each cell in the graph you should change the value to ERR
    - similarly you should take this value ERR and instead of doing a toposort in the affected cells, you should directly keep a set of the affected cells and change their value to ERR, i think that would be time efficient

- update_dependents(curr_cell, dependents) -> forms the DAG for the affected cells and then update them as mentioned below
    TopoSort:
    - each cell has an attribute needs_update (= true)
    - while iterating over the dependents of the current cell, you need to add each element to the graph after checking if it needs a change
    - then you should form the graph of the dependents(the affected cells)
    - then update the values of the affected cells in the topological order
    - if at any point you encounter a div by zero error, you need to mark the value of each cell in the affected cells graph to ERR
    - after updating, change the needs_update attribute of each affected cell to be true again

- this way you update the values of the affected cells incorporating an update in the spreadsheet

For a cell:
- you need a `dependency set`, `dependent set`
- a `needs_update` attribute initialised to false
- `value` attribute to hold the arithmetic value

