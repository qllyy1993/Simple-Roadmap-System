#include "roadMap.h"
#include <iostream>
#include <vector>
#include <list>
#include <map>
#include <string>
#include <queue>          // std::priority_queue
#include <functional>     // std::greater
#include <stack>          // std::stack
#include <algorithm>      //std::find
#include <array>          //std::array
#include <set>           //std::set

//using namespace std;

//declare functions
unsigned int split(const std::string &txt, std::vector<std::string> &strs, string delimiter);
void Dijkastra (unsigned int fromVertex, unsigned int toVertex, adjacencyList& al , stack<int>& pathConstruct);
void pathDisplay(unsigned int fromVertex, unsigned int toVertex, stack<int>& pathConstruct, 
				 vector<unsigned int>& shortestPathVertex, vector<unsigned int>& shortestPathEdge, 
				 map<unsigned int, string>& edgeOnRoad);
void removeEdgeInAdjL(unsigned int vertex1, unsigned int vertex2,  adjacencyList& al);

// first element of pair: distance of a vertex, second element of pair: index of a vertex
typedef pair<float, int>  distVertPair; 

//total number of vertices
static unsigned int verNum=0; 

//map a vertex's index to its label 
vector<string> index2Label; 

//road name -> edges forming this road
typedef map<string, vector<string> > roadEdges;

//edges constituting this road-> road name
map<vector<int>, string> roadsMap;
map<int, string> edge2Road; //map each road to its road name, if it has one.

//edge -> event on it
map<unsigned int, EventType> eventsMap;

//map a vertex's label to its index
typedef map<string, Vertex> labelMap;
labelMap label2Vertex;

// used to store edges, and let the edge ID sequential
vector<Edge> edgeVector; 

//map <startVertex,endVertex> to edge ID
map<pair<int,int>,int> edgeMap; 

RoadMap::RoadMap()
{ 
	// clear current state

	//static int
	verNum=0;

	//vectors
	index2Label.clear();
	edgeVector.clear();
	adjL.clear();
	adjL.resize(10);

	//maps
	roadsMap.clear();
	eventsMap.clear();
	label2Vertex.clear();
	edgeMap.clear();
}
RoadMap::~RoadMap()
{ 
	adjL.clear();
}

void RoadMap::addVertex(VertexType type, string label)
{
	//check if the vertex is already exist
	labelMap::iterator it= label2Vertex.find(label);
	if (it != label2Vertex.end()){
		cout<<"This vertex already EXISTS"<<endl;
		return;
	}

	Vertex v(verNum++,type,label);
	label2Vertex[v.label]=v;
	index2Label.push_back(v.label);

	//enlarge the adjacency list by 10 if there are too many vertices
	if (verNum>adjL.size())
	{
		adjL.resize(2*adjL.size());
	}
}

void RoadMap::addEdge(unsigned int vertex1, unsigned int vertex2, bool directional, float speed, float length)
{
	//check whether vertex1 and vertex2 are known
	if (vertex1>=index2Label.size())
	{
		std::cout<<"Failed to add an edge from "<<vertex1<<" to "<<vertex2
			<<", because " <<vertex1<<" is not known"<<endl;
		return;
	}

	if (vertex2>=index2Label.size())
	{
		std::cout<<"Failed to add an edge from "<<vertex1<<" to "<<vertex2
			<<", because " <<vertex2<<" is not known"<<endl;
		return;
	}

	// vertex1 and vertex2 are known, it is time to add an edge between them
	Edge e(edgeVector.size(),vertex1,vertex2, directional, speed, length);
	edgeVector.push_back(e);

	edgeMap[make_pair(vertex1,vertex2)]=e.ID;

	//add edge to adjacency list
	vector<float> weight(2);
	weight[0]=speed; 
	weight[1]=length;

	adjL[vertex1].push_back(make_pair(vertex2, weight));
	if (!directional) 
		adjL[vertex2].push_back(make_pair(vertex1, weight));
}

void RoadMap::edgeEvent(unsigned int edge, EventType eventType)
{

	//check if the edge is known
	if (edge>=edgeVector.size())
	{
		std::cout<<"Failed to add an event on edge "<<edge
			<<", because edge " <<edge<<" is not known"<<endl;
		return;
	}

	eventsMap[edge]=eventType;
}

void RoadMap::removeEvent(unsigned int edge, EventType eventType)
{
	if (edge>=edgeVector.size())
	{
		std::cout<<"Failed to remove event on edge "<<edge
			<<", because edge " <<edge<<" is not known"<<endl;
		return;
	} 

	if (eventsMap[edge]!=eventType)
	{
		std::cout<<"There isn't this type of event on edge "<<edge;
		return;
	}

	eventsMap.erase(edge);
}

void RoadMap::road(vector<int> edges, string name)
{
	//map the edge vector to name, used in store and retrieve API
	roadsMap[edges]=name;

	//map each edge on this road to the road name, used to find the road name of edges in shortest path
	for (std::vector<int>::iterator it = edges.begin() ; it != edges.end(); ++it){
		edge2Road[*it]=name;
	}
}

void RoadMap::trip(Vertex fromVertex, Vertex toVertex) 
{
	//check whether the fromVertex and toVertex are known
	if (static_cast<size_t>(fromVertex.index)>=index2Label.size())
	{
		cout<<"Failed to find a path from "<<fromVertex.label<<" to "<<toVertex.label
			<<", because " <<fromVertex.label<<" is not known"<<endl;
		return;
	}

	if (toVertex.index>=index2Label.size())
	{
		cout<<"Failed to find a path from "<<fromVertex.label<<" to "<<toVertex.label
			<<", because " <<toVertex.label<<" is not known"<<endl;
		return;
	}

	adjacencyList adjLOrigin=this->adjL; //copy the adjacentcy list before some edges might be removed
	char flag=0;	//set this flag if the shortest path contains events
	do{
		flag=0;

		//call Dijkastra function and store the shorest path in a stack
		stack<int> pathConstruct;
		Dijkastra (fromVertex.index, toVertex.index, this->adjL, pathConstruct);

		//no path from "fromVertex" to "toVertex"
		if (pathConstruct.size()==0){
			cout<<"There is no other path from "<<fromVertex.index<<" to "<<toVertex.index<<endl;
			return;
		}

		//represent the shortest path using a sequence of vertices and edges and output them to the terminal or console
		vector<unsigned int> sPV;
		vector<unsigned int> sPE;
		map<unsigned int, string> eOR;
		pathDisplay(fromVertex.index, toVertex.index, pathConstruct, sPV, sPE, eOR);

		//push them in public parameters
		shortestPathVertex.push_back(sPV);
		shortestPathEdge.push_back(sPE);
		edgeOnRoad.push_back(eOR);

		//check if there are events on the shortest path, If there is, remove all edges with event from adjacency list and find path again
		vector<unsigned int>::iterator it;
		map<unsigned int, EventType>::iterator eventIt;  //edge -> event on it
		for(it=sPE.begin() ;it!=sPE.end();++it)
		{
			eventIt=eventsMap.find(*it);
			if (eventIt!= eventsMap.end())
			{
				flag=1;
				cout<<"Event "<<eventIt->second<<" on edge "<<eventIt->first<<", ";
				//erase the edge with event in adjancency list
				removeEdgeInAdjL(edgeVector[eventIt->first].startVertex, edgeVector[eventIt->first].endVertex, this->adjL);
			}
		}
		cout<<endl;
	}while(flag==1);

	this->adjL=adjLOrigin;
}

Vertex RoadMap::vertex(string point_of_interest)
{
	labelMap::iterator it= label2Vertex.find(point_of_interest);
	if (it != label2Vertex.end()){
		return label2Vertex.find(point_of_interest)->second;
	}
	else{
		cout<< point_of_interest<<" is not known!"<<endl;
		Vertex v(INT_MAX,POI,"NOT KNOWN");
		return v;

	}
}

void RoadMap::store(string filename)
{
	fileWritor.open (filename);
	if (fileWritor.is_open())
	{
		// Print all vertices
		fileWritor <<"// Vertices:"<<endl
			<<"// ID, Type, Label"<<endl;
		for (unsigned int i = 0; i <verNum; i++) {
			fileWritor <<i<<", "         //ID
				<< label2Vertex.find(index2Label[i])->second.type<<", "    //type
				<< label2Vertex.find(index2Label[i])->second.label<<endl;  //label
		}
		fileWritor<<endl;

		// Print all edges
		fileWritor <<"// Edges:"<<endl
			<<"// ID, StartVertex, EndVertex, Directional, Speed, Length"<<endl;
		for (unsigned int i = 0; i <edgeVector.size(); i++) {
			fileWritor <<i<<", "         //ID
				<< edgeVector[i].startVertex<<", "   
				<< edgeVector[i].endVertex<<", "   
				<< edgeVector[i].directional<<", "   
				<< edgeVector[i].speed<<", "   
				<< edgeVector[i].length<<endl; 
		}
		fileWritor<<endl;

		// Print all events
		fileWritor <<"// Events:"<<endl
			<<"// Edge, Type"<<endl;
		for (map<unsigned int, EventType>::iterator it=eventsMap.begin(); it!=eventsMap.end(); ++it)
		{
			fileWritor<< it->first << ", "  //edge
				<< it->second << endl;      //type

		}
		fileWritor<<endl;

		// Print all roads
		fileWritor <<"// Roads:"<<endl
			<<"// Edges[], Name"<<endl;
		for (map<vector<int>, string>::iterator it=roadsMap.begin(); it!=roadsMap.end(); ++it)
		{
			for (unsigned int i=0; i<it->first.size();i++)  //edges[]
				fileWritor<<it->first[i]<<", ";
			fileWritor << it->second << endl;      //name of the road
		}

		fileWritor.close();
	}
	else 
		cout << "Unable to open file "<<filename<<endl; 
}

void RoadMap::retrieve(string filename)
{
	RoadMap::RoadMap();
	string line;
	vector<string> v;
	vector<int> edges;

	fileReader.open (filename);
	if (fileReader.is_open())
	{
		try {

			//read Vertices
			getline (fileReader,line);
			while(!line.empty()){
				if (line[0] != '/'){ //not the two comment lines at the top				
					split( line, v, ", " );
					RoadMap::addVertex(static_cast<VertexType>(stoi(v[1])),v[2]);		
				}
				getline (fileReader,line);
			}

			//read Edges
			getline (fileReader,line);
			while(!line.empty()){
			if (line[0] != '/'){ //not the two comment lines at the top
					split( line, v, ", " );
					RoadMap::addEdge(stoi(v[1]), stoi(v[2]), !!(stoi(v[3])),stof(v[4]), stof(v[5]));
				}
				getline (fileReader,line);
			}

			//read Events
			getline (fileReader,line);
			while(!line.empty()){
				if (line[0] != '/'){ //not the two comment lines at the top
					split( line, v, ", " );
					RoadMap::edgeEvent(stoi(v[0]), static_cast<EventType>(stoi(v[1])));			
				}
				getline (fileReader,line);
			}

			//read Roads
			getline (fileReader,line);
			while(!line.empty()){
				if (line[0] != '/'){ //not the two comment lines at the top
					split( line, v, ", " );
					edges.clear();
					for(unsigned int i=0;i<v.size()-1;i++){
						edges.push_back(stoi(v[i]));
					}
					RoadMap::road(edges, v[v.size()-1]);		
					//cout<< v[v.size()-1]<<endl;
				}
				getline (fileReader,line);
			}

		}
		catch (const std::invalid_argument& ia){
			std::cerr << "Invalid argument: " << ia.what() << '\n';
		}
		catch (const std::out_of_range& oor){
			std::cerr << "Out of Range error: " << oor.what() << '\n';
			cout<<"please delete blank lines at the end of "<<filename<<endl;
		}

		fileReader.close();

	}
	else 
		cout << "Unable to open file "<<filename<<endl; 
}

unsigned int split(const std::string &txt, std::vector<std::string> &strs, string delimiter)
{
	size_t pos = txt.find( delimiter);
	unsigned int initialPos = 0;
	strs.clear();

	// Decompose statement
	while( pos!= string::npos ) {
		strs.push_back( txt.substr( initialPos, pos - initialPos + 1 ) );
		initialPos = pos + delimiter.length();

		pos = txt.find( delimiter, initialPos );
	}

	// Add the last one
	strs.push_back( txt.substr( initialPos, min(pos, txt.size() ) - initialPos + 1 ) );

	return strs.size();
}

void Dijkastra (unsigned int fromVertex, unsigned int toVertex, adjacencyList& al , stack<int>& pathConstruct)
{
	// Create a priority queue to store vertices that are being preprocessed. This is weird syntax in C++.
	priority_queue< distVertPair, vector <distVertPair> , greater<distVertPair> > pq;

	// Create a vector for distances and initialize all
	// distances as infinite (INF)
	vector<float> dist(al.size(), numeric_limits<float>::max());
	vector<int> prev(al.size(), INT_MAX);

	// Insert source itself in priority queue and initialize
	// its distance as 0.
	pq.push(make_pair(0, fromVertex));
	dist[fromVertex] = 0;

	/* Looping till priority queue becomes empty (or all
	distances are not finalized) */
	int u; //current vertex poped up from priority queue
	int v; //current out-neighbour of u 
	float length; //edge length of u->v
	while (!pq.empty())
	{
		// The first vertex in pair is the minimum distance
		// vertex, extract it from priority queue.
		// vertex label is stored in second of pair (it
		// has to be done this way to keep the vertices
		// sorted distance (distance must be first item
		// in pair)
		u = pq.top().second;
		if (u==toVertex)
			break;
		pq.pop();

		// 'itr' is used to get all out-neighbours of u
		singleAdjL::iterator itr; 
		for (itr = al[u].begin(); itr != al[u].end(); ++itr)
		{
			// Get vertex index and edge length of current neighbour of u, name it as v.
			v = (*itr).first;
			length = (*itr).second[1]; //.second[0] is speed, and .second[1] is length

			//  If there is shorted path to v through u.
			if (dist[v] > dist[u] + length)
			{
				// Updating distance of v
				dist[v] = dist[u] + length;
				prev[v] = u;
				pq.push(make_pair(dist[v], v));
			}
		}
	}

	//check if there is a path from "fromVertex" to "toVertex" 
	if (prev[toVertex]==INT_MAX){
		return;
	}

	//read the shortest path from "fromVertex" to "toVertex" by reverse iteration
	//the first u equals "toVertex" when the program break from last while loop.
	while (prev[u]!=INT_MAX) 
	{
		pathConstruct.push(u);
		u=prev[u]; // Traverse from target to source
	}
	//push "fromVertex" into the stack
	pathConstruct.push(fromVertex);
}

void pathDisplay(unsigned int fromVertex, unsigned int toVertex, stack<int>& pathConstruct,
				 vector<unsigned int>& shortestPathVertex, vector<unsigned int>& shortestPathEdge,
				 map<unsigned int, string>& edgeOnRoad)
{
	//use a sequence of vertices to represent the short path
	while (!pathConstruct.empty())
	{
		shortestPathVertex.push_back(pathConstruct.top()); 
		pathConstruct.pop();
	}

	//output the sequence of vertices to screen 
	cout<<"trip---Shortest path from "<<fromVertex<<" to "<<toVertex<<" (Vertex ID): ";
	vector<unsigned int>::iterator it;
	for(it=shortestPathVertex.begin() ;it!=shortestPathVertex.end();++it)
		cout <<*it<<", ";
	cout<<";";

	//use a sequence of edges to represent the short path
	for(it=shortestPathVertex.begin() ;it!=shortestPathVertex.end()-1;++it)
	{
		if (edgeMap.find(make_pair(*it,*(it+1)))!=edgeMap.end())
			shortestPathEdge.push_back(edgeMap[make_pair(*it,*(it+1))]);
		else //the edge must be bidirectional and the start vertex is *(it+1), not *it
			shortestPathEdge.push_back(edgeMap[make_pair(*(it+1),*it)]);
	}

	//add edges and their road names to a map
	for(it=shortestPathEdge.begin() ;it!=shortestPathEdge.end();++it)
	{
		if (edge2Road.find(*it)!=edge2Road.end())
			edgeOnRoad[*it]=edge2Road[*it];
		else //this edge doesn't have a road name
			edgeOnRoad[*it]="";
	}

	//output the sequence of edges and their road name to screen 
	cout<<"(Edges ID): ";
	for(it=shortestPathEdge.begin() ;it!=shortestPathEdge.end();++it)
		cout <<*it<<" "<<edgeOnRoad[*it]<<", ";
	cout<<endl;
}

void removeEdgeInAdjL(unsigned int vertex1, unsigned int vertex2,  adjacencyList& al)
{
	//remove edge from adjacency list
	vector<float> weight(2);
	weight[0]=edgeVector[edgeMap[make_pair(vertex1,vertex2)]].speed; 
	weight[1]=edgeVector[edgeMap[make_pair(vertex1,vertex2)]].length;

	al[vertex1].remove(make_pair(vertex2, weight));
	if (edgeVector[edgeMap[make_pair(vertex1,vertex2)]].directional==false) 
		al[vertex2].remove(make_pair(vertex1, weight));
}
