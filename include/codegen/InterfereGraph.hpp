#pragma once

#include "DynamicBitset.hpp"
#include "LiveMessage.hpp"

struct MoveInstNode;
class MCopy;
class GraphColorSolver;

struct InterfereGraphNode
{
	InterfereGraphNode* parent_;
	InterfereGraphNode* pre_;
	InterfereGraphNode* next_;
	RegisterLike* reg_;
	std::vector<MoveInstNode*> moveList_;
	std::vector<InterfereGraphNode*> adjList_;
	unsigned degree_ = 0;
	float weight_ = 0;
	InterfereGraphNode* alias_ = nullptr;
	Register* color_ = nullptr;
};

struct MoveInstNode
{
	MoveInstNode* parent_;
	MoveInstNode* pre_;
	MoveInstNode* next_;
	MCopy* inst_;
};

class AdjSet : public DynamicBitset
{
	unsigned len_;

public:
	AdjSet(unsigned len) : DynamicBitset(len * len)
	{
		len_ = len;
	}

	AdjSet(int len) : DynamicBitset(len * len)
	{
		len_ = len;
	}

	AdjSet(): len_(0)
	{
	}

	[[nodiscard]] bool test(int i, int j) const
	{
		return DynamicBitset::test(i * len_ + j);
	}

	[[nodiscard]] bool test(unsigned i, unsigned j) const
	{
		return DynamicBitset::test(i * len_ + j);
	}

	void set(int i, int j) const
	{
		DynamicBitset::set(i * len_ + j);
	}

	void set(unsigned i, unsigned j) const
	{
		DynamicBitset::set(i * len_ + j);
	}
};

class InterfereGraph
{
	GraphColorSolver* parent_;
	std::unordered_map<RegisterLike*, InterfereGraphNode*> regNodes_;
	std::unordered_map<MCopy*, MoveInstNode*> moveNodes_;
	MoveInstNode* worklistMoves_;
	MoveInstNode* activeMoves_;
	MoveInstNode* coalescedMoves_;
	MoveInstNode* constrainedMoves_;
	MoveInstNode* frozenMoves_;
	InterfereGraphNode* initial_;
	InterfereGraphNode* spillWorklist_;
	InterfereGraphNode* freezeWorklist_;
	InterfereGraphNode* simplifyWorklist_;
	InterfereGraphNode* coalescedNodes_;
	InterfereGraphNode* selectStack_;
	InterfereGraphNode* spilledNode_;
	AdjSet adjSet_;
	unsigned K_ = 0;

	InterfereGraphNode* getOrCreateRegNode(RegisterLike* reg);
	MoveInstNode* getOrCreateMoveNode(MCopy* reg);
	void addEdge(unsigned i, unsigned j);
	bool moveRelated(const InterfereGraphNode* node) const;
	void decrementDegree(InterfereGraphNode* node) const;
	InterfereGraphNode* getAlias(InterfereGraphNode* node);
	void addWorkList(InterfereGraphNode* node) const;
	bool ok(const InterfereGraphNode* t, const InterfereGraphNode* u) const;
	bool allOk(const InterfereGraphNode* u, const InterfereGraphNode* v) const;
	[[nodiscard]] bool conservative(const std::vector<InterfereGraphNode*>& nodes,
	                                const std::vector<InterfereGraphNode*>& nodes2) const;
	void combine(InterfereGraphNode* u, InterfereGraphNode* v);
	void enableMoves(const InterfereGraphNode* u) const;
	int idOf(const InterfereGraphNode* n) const;
	void freezeMove(InterfereGraphNode* u);
	bool nodeMoveInMoveList(const MoveInstNode* n) const;
	bool emptyNodeMove(const InterfereGraphNode* n) const;
	InterfereGraphNode* xOf(const MoveInstNode* m);
	InterfereGraphNode* yOf(const MoveInstNode* m);

public:
	InterfereGraph(const InterfereGraph& other) = delete;
	InterfereGraph(InterfereGraph&& other) = delete;
	InterfereGraph& operator=(const InterfereGraph& other) = delete;
	InterfereGraph& operator=(InterfereGraph&& other) = delete;

	~InterfereGraph();
	InterfereGraph(GraphColorSolver* parent);
	void build();
	void flush();
	void makeWorklist() const;
	[[nodiscard]] bool shouldRepeat() const;
	[[nodiscard]] bool simplify() const;
	bool coalesce();
	bool freeze();
	bool selectSpill();
	void assignColors();
	[[nodiscard]] bool needRewrite() const;
	void rewriteProgram() const;
	void applyChanges();
};

void calculateFrameInterfereGraph(const GraphColorSolver* parent, const FrameLiveMessage& flm);
