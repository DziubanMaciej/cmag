#include "graph_layout.h"

#include "cmag_core/utils/error.h"

#include <memory>

struct Node {
    Node() = default;
    Node(const Node &) = delete;
    Node(Node &&) = delete;

    CmagTarget *target = nullptr;
    bool isOrphan = false;

    // Each step may need to associate some amount of metadata with each node.
    // This union is a place for such temporary data. It should be valid only
    // in currently executed step.
    union {
        struct {
            bool removed = false;
        } assignLayers;
    } data = {};
};

struct Edge {
    Node *a = nullptr;
    Node *b = nullptr;
};

struct Layer {
    std::vector<Node *> nodes = {};
};

struct Graph {
    std::unique_ptr<Node[]> nodes = {};
    size_t nodesCount = 0u;
    std::vector<Edge> edges = {};
    std::vector<Layer> layers = {};
    Layer orphansLayer = {};
};

static Graph createGraph(const std::vector<CmagTarget *> &targets, std::string_view configName) {
    Graph graph = {};

    // Initialize nodes
    graph.nodesCount = targets.size();
    graph.nodes = std::make_unique<Node[]>(graph.nodesCount);
    for (size_t nodeIndex = 0u; nodeIndex < graph.nodesCount; nodeIndex++) {
        graph.nodes[nodeIndex].target = targets[nodeIndex];

        const bool hasIngoingEdges = targets[nodeIndex]->derived.isReferenced;
        const bool hasOutgoingEdges = targets[nodeIndex]->tryGetConfig(configName)->derived.allDependencies.size() > 0;
        if (!hasIngoingEdges && !hasOutgoingEdges) {
            graph.nodes[nodeIndex].isOrphan = true;
        }
    }

    auto findNode = [&graph](const CmagTarget *target) {
        for (size_t nodeIndex = 0u; nodeIndex < graph.nodesCount; nodeIndex++) {
            Node &node = graph.nodes[nodeIndex];
            if (node.target == target) {
                return &node;
            }
        }
        UNREACHABLE_CODE;
    };

    // Initialize edges
    for (const CmagTarget *srcTarget : targets) {
        for (const CmagTarget *dstTarget : srcTarget->tryGetConfig(configName)->derived.allDependencies) {
            Node *srcNode = findNode(srcTarget);
            Node *dstNode = findNode(dstTarget);
            graph.edges.push_back({srcNode, dstNode});
        }
    }

    return graph;
}

static void assignLayersTopological(Graph &graph) {
    // Initialize all nodes to be 'not removed'. We will be removing them later in the process.
    // Mark all orphans in the graph as 'removed'. We will not assign them to any layers
    // and they will be assigned positions separately.
    for (size_t nodeIndex = 0u; nodeIndex < graph.nodesCount; nodeIndex++) {
        Node &node = graph.nodes[nodeIndex];
        node.data.assignLayers.removed = node.isOrphan;
        if (node.isOrphan) {
            graph.orphansLayer.nodes.push_back(&node);
        }
    }

    // A helper function to gather all nodes that are not pointed to by any edge. They will be
    // our next layer.
    auto getUnreferencedNodes = [&]() {
        std::vector<Node *> result = {};
        for (size_t nodeIndex = 0u; nodeIndex < graph.nodesCount; nodeIndex++) {
            Node &node = graph.nodes[nodeIndex];
            if (node.data.assignLayers.removed) {
                continue;
            }

            bool isNodeReferenced = false;
            for (Edge &edge : graph.edges) {
                if (!edge.a->data.assignLayers.removed &&
                    !edge.b->data.assignLayers.removed &&
                    edge.b == &node) {
                    isNodeReferenced = true;
                    break;
                }
            }

            if (!isNodeReferenced) {
                result.push_back(&node);
            }
        }

        return result;
    };

    auto unreferencedNodes = getUnreferencedNodes();
    while (!unreferencedNodes.empty()) {
        // Add all unreferenced nodes as a new layer.
        graph.layers.push_back(Layer{unreferencedNodes});

        // Remove all unreferenced node from the graph, so that we won't look at them later.
        // They are already processed in layer assignment step.
        for (Node *node : unreferencedNodes) {
            node->data.assignLayers.removed = true;
        }

        // Query unreferenced nodes again. The result will be different than the last time,
        // because we removed some nodes and edges.
        unreferencedNodes = getUnreferencedNodes();
    }
}

static void assignCoordinates(Graph &graph, size_t nodeWidth, size_t nodeHeight) {
    for (size_t layerIndex = 0u; layerIndex < graph.layers.size(); layerIndex++) {
        Layer &layer = graph.layers[layerIndex];

        const float paddingPercentageVertical = 2.5f;
        const float y = layerIndex * nodeHeight * paddingPercentageVertical;

        for (size_t nodeIndex = 0u; nodeIndex < layer.nodes.size(); nodeIndex++) {
            Node &node = *layer.nodes[nodeIndex];
            const float paddingPercentageHorizontal = 1.4f;
            const float x = nodeIndex * paddingPercentageHorizontal * nodeWidth;

            CmagTargetGraphicalData &graphical = node.target->graphical;
            graphical.x = x;
            graphical.y = y;
        }
    }

    for (size_t nodeIndex = 0u; nodeIndex < graph.orphansLayer.nodes.size(); nodeIndex++) {
        Node &node = *graph.orphansLayer.nodes[nodeIndex];
        CmagTargetGraphicalData &graphical = node.target->graphical;

        graphical.x = -100;
        graphical.y = nodeIndex * nodeHeight * 1.05f;
    }
}

void calculateLayout(const std::vector<CmagTarget *> &targets,
                     std::string_view configName,
                     size_t nodeWidth,
                     size_t nodeHeight) {
    Graph graph = createGraph(targets, configName);
    assignLayersTopological(graph);
    assignCoordinates(graph, nodeWidth, nodeHeight);
}
