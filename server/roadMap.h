#ifndef ROADMAP_H
#define ROADMAP_H

#include <vector>
#include <list>
#include <string>
#include <fstream>// file stream
#include <map>    

using namespace std;

enum VertexType {POI, INTERSECTION};
enum EventType {ACCIDENT, CARSTOPED,DEBRIS,CLOSURE};

// Adjacency List is a vector of list.
// Where each element is a pair<int, vector<float>>.
// pair.first -> the index of edge's destination vertex.
// pair.second -> edge's weight, namely speed and length.
typedef list< pair<int, vector<float>>> singleAdjL;
typedef vector<singleAdjL> adjacencyList; 

class Vertex {
public:
	unsigned int index;
	VertexType type;
	string label;

	Vertex(){ };
	Vertex(unsigned int index,VertexType type,string label):
		index(index),type(type),label(label){ };
	~Vertex(){ };
};

class Edge {  
public:
	unsigned int ID;
	unsigned int startVertex;
	unsigned int endVertex;
	bool directional;
	float speed, length;
	Edge(unsigned int ID, unsigned int startVertex, unsigned int endVertex, bool directional, float speed, float length):
		ID(ID),startVertex(startVertex),endVertex(endVertex),directional(directional),speed(speed),length(length){ };
	~Edge(){ };
};

class RoadMap
{
public:
	RoadMap();
	~RoadMap();

	void addVertex(VertexType type, string label);

	//directional: TRUE=directional FALSE=non-directional
	void addEdge(unsigned int vertex1, unsigned int vertex2, bool directional, float speed, float length);
	
	void edgeEvent(unsigned int edge, EventType eventType);

	void removeEvent(unsigned int edge, EventType eventType);
		
	void road(vector<int> edges, string name);

	void trip(Vertex fromVertex, Vertex toVertex);
	
	Vertex vertex(string point_of_interest);
	
	void store(string filename);
	
	void retrieve(string filename);

private:
	adjacencyList adjL;  //used to find shortest path
	ofstream fileWritor; //write on files
	ifstream fileReader; //read on files

public:
	vector<vector<unsigned int>> shortestPathVertex;//use a sequence of vertices to represent the short path	
	vector<vector<unsigned int>> shortestPathEdge;  //use a sequence of edges to represent the short path
	vector<map<unsigned int, string>> edgeOnRoad;  //store the road name of edges making up of short path
};

#endif