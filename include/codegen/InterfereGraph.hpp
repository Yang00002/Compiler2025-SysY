#pragma once

#include "DynamicBitset.hpp"
#include "LiveMessage.hpp"

struct MoveInstNode;
class MCopy;
class GraphColorSolver;

struct InterfereGraphNode
{
	InterfereGraphNode* parent_ = nullptr;
	InterfereGraphNode* pre_ = this;
	// 节点所属的集合
	InterfereGraphNode* next_ = this;
	// 节点寄存器
	RegisterLike* reg_;
	// 与该节点相关的传送指令
	std::vector<MoveInstNode*> moveList_;
	// 对虚拟寄存器, 是与节点冲突的节点集合
	std::vector<InterfereGraphNode*> adjList_;
	// 节点的冲突节点数
	int degree_ = 0;
	// 指令的重要性
	float weight_ = 0;
	// 代表它的节点(尤其当其被合并后)
	InterfereGraphNode* alias_ = nullptr;
	// 颜色
	Register* color_ = nullptr;

	void add(InterfereGraphNode* target);
	void remove(InterfereGraphNode* target) const;
	void clear();
	bool contain(const InterfereGraphNode* target) const;
	[[nodiscard]] bool preColored() const;
	[[nodiscard]] bool empty() const;
	[[nodiscard]] InterfereGraphNode* top() const;
	// 获取代表节点, 特别是当已经合并后
	InterfereGraphNode* alias();
};

struct MoveInstNode
{
	MoveInstNode* parent_ = nullptr;
	MoveInstNode* pre_ = this;
	MoveInstNode* next_ = this;
	InterfereGraphNode* toX_;
	InterfereGraphNode* fromY_;

	void add(MoveInstNode* target);
	void remove(MoveInstNode* target) const;
	void clear();
	bool contain(const MoveInstNode* target) const;
	[[nodiscard]] bool empty() const;
	[[nodiscard]] MoveInstNode* top() const;
};

class AdjSet : public DynamicBitset
{
	int len_;

public:
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

	void set(int i, int j)
	{
		DynamicBitset::set(i * len_ + j);
	}
};

class InterfereGraph
{
	GraphColorSolver* parent_;
	std::unordered_map<RegisterLike*, InterfereGraphNode*> regNodes_;
	std::unordered_map<MCopy*, MoveInstNode*> moveNodes_;
	// 有可能合并的传送指令集合
	MoveInstNode worklistMoves_;
	// 还未做好合并准备的传送指令集合
	MoveInstNode activeMoves_;
	// 已经合并的传送指令集合
	MoveInstNode coalescedMoves_;
	// 源和目标操作数冲突的传送指令集合
	MoveInstNode constrainedMoves_;
	// 不再考虑合并的传送指令集合
	MoveInstNode frozenMoves_;
	// 初始化的虚拟寄存器集合, 初始包含所有虚拟寄存器
	InterfereGraphNode initial_;
	// 要溢出(高度数) 节点表
	InterfereGraphNode spillWorklist_;
	// 可能要冻结(低度数, 传送有关) 节点表 -> (因为不一定有度数, 所以也有可能需要合并)
	InterfereGraphNode freezeWorklist_;
	// 要简化(低度数, 传送无关) 节点表
	InterfereGraphNode simplifyWorklist_;
	// 已经合并的寄存器集合 (u v 合并后, u 代表 v, 而 v 放入该表)
	InterfereGraphNode coalescedNodes_;
	// 删除的虚拟寄存器的栈
	InterfereGraphNode selectStack_;
	// 实际要溢出的节点集合
	InterfereGraphNode spilledNode_;
	// 冲突边, 描述节点间冲突关系
	AdjSet adjSet_;
	// 可用物理寄存器数量
	int K_ = 0;

	InterfereGraphNode* getOrCreateRegNode(RegisterLike* reg);
	MoveInstNode* getOrCreateMoveNode(MCopy* reg);
	// 添加冲突边(操作数可以是源, 可以是目标)
	void addEdge(int i, int j);
	// 添加冲突边(操作数可以是源, 可以是目标), 但是不增加一端的度数
	void combineAddEdge(int i, int noAdd);
	// 节点是否传送相关
	bool moveRelated(const InterfereGraphNode* node) const;
	// 使节点的度数减少 1, 并自动重新分类(不用对物理寄存器进行, 也没有效果)
	void decrementDegree(InterfereGraphNode* node);
	// 若节点为虚拟寄存器, 低度数, 并且变得传送无关, 则从冻结表(低度数, 传送有关)转移到简化表 
	void addWorkList(InterfereGraphNode* node);
	// George 合并测试(测试一个虚拟和一个物理寄存器, u 邻居要么与 v 冲突, 要么是低度数)
	bool GeorgeTest(const InterfereGraphNode* virtualReg, const InterfereGraphNode* physicReg) const;
	// Briggs 合并测试(测试两个虚拟寄存器, 合并后度数高的节点小于 K)
	[[nodiscard]] bool BriggsTest(const InterfereGraphNode* l, const InterfereGraphNode* r) const;
	// 指令是否是有价值的传送指令(是 MCOPY, 操作数都是关注的寄存器, 至少有一个虚拟寄存器)
	bool isCareCopy(MInstruction* inst) const;
	// 和并节点 v 到 u
	void combine(InterfereGraphNode* u, InterfereGraphNode* v);
	int regIdOf(const InterfereGraphNode* n) const;
	// 将节点相关未做好准备的传送指令放入可能合并的工作表
	void enableMoves(const InterfereGraphNode* u);
	// 冻结节点相关传送指令
	void freezeMove(InterfereGraphNode* u);
	bool inGraph(const MoveInstNode* n) const;
	// 节点是否仍处于冲突图中(未被合并, 不在栈中)
	bool inGraph(const InterfereGraphNode* n) const;

	bool adj(const InterfereGraphNode* l, const InterfereGraphNode* r) const;

public:
	InterfereGraph(const InterfereGraph& other) = delete;
	InterfereGraph(InterfereGraph&& other) = delete;
	InterfereGraph& operator=(const InterfereGraph& other) = delete;
	InterfereGraph& operator=(InterfereGraph&& other) = delete;

	~InterfereGraph();
	InterfereGraph(GraphColorSolver* parent);
	// 构建冲突图, 初始化有可能合并的传送指令集合
	void build();
	void flush();
	// 给初始节点分类
	void makeWorklist();
	[[nodiscard]] bool shouldRepeat() const;
	// 简化, 去除低度数传送无关节点
	[[nodiscard]] bool simplify();
	bool coalesce();
	// 尝试冻结节点, 不再尝试合并
	bool freeze();
	bool selectSpill();
	void assignColors();
	[[nodiscard]] bool needRewrite() const;
	void rewriteProgram() const;
	void applyChanges();
};

void calculateFrameInterfereGraph(const GraphColorSolver* parent, const FrameLiveMessage& flm);
