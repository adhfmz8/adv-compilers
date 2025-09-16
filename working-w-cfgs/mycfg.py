import json
import sys
from collections import OrderedDict, defaultdict
from collections import deque


TERMINATORS = "jmp", "br", "ret"


def form_blocks(body):
    curr_block = []
    for instr in body:
        if "label" in instr:
            if curr_block:
                yield curr_block
            curr_block = [instr]
        else:
            curr_block.append(instr)
            if "op" in instr and instr["op"] in TERMINATORS:
                yield curr_block
                curr_block = []

    if curr_block:
        yield curr_block


def block_map(blocks):
    out = OrderedDict()

    for block in blocks:
        if "label" in block[0]:
            name = block[0]["label"]
            block = block[1:]
        else:
            name = "b{}".format(len(out))
        out[name] = block

    return out


def get_cfg(name2block):
    out = {}
    for i, (name, block) in enumerate(name2block.items()):
        if not block:
            succ = []
            out[name] = succ
            continue
        last = block[-1]
        if last.get("op") in ("jmp", "br"):
            succ = last["labels"]
        elif last.get("op") == "ret":
            succ = []
        else:
            if i == len(name2block) - 1:
                succ = []
            else:
                succ = [list(name2block.keys())[i + 1]]
        out[name] = succ
    return out


def get_path_lengths(cfg, entry):
    """
    Compute the shortest path length (in edges) from the entry node to each node in the CFG.

    Parameters:
    cfg (dict): mapping {node: [successors]}
    entry (str): starting node

    Returns:
    dict: {node: distance from entry}, unreachable nodes are omitted
    """
    dist = {node: float("inf") for node in cfg}
    dist[entry] = 0
    q = deque([entry])

    while q:
        curr = q.popleft()
        for v in cfg[curr]:
            if dist[v] == float("inf"):
                dist[v] = dist[curr] + 1
                q.append(v)

    return dist


def reverse_postorder(cfg, entry):
    """
    Compute reverse postorder (RPO) for a CFG.

    Parameters:
    cfg (dict): mapping {node: [successors]}
    entry (str): starting node

    Returns:
    list: nodes in reverse postorder
    """
    visited = set()
    order = []

    def dfs(node):
        if node in visited:
            return
        visited.add(node)
        for succ in cfg.get(node, []):
            dfs(succ)
        order.append(node)

    dfs(entry)
    return list(reversed(order))


def find_back_edges(cfg, entry):
    """
    Find back edges in a CFG using DFS.

    Parameters:
    cfg(dict): mapping {node: [successors]}
    entry(str): starting node

    Returns: list of edges (u,v) where u->v is a back edge
    """
    color = {node: 0 for node in cfg}
    back_edges = []

    def dfs(u):
        color[u] = 1
        for v in cfg[u]:
            if color[v] == 0:
                # unexplored edge
                dfs(v)
            elif color[v] == 1:
                back_edges.append((u, v))
            # done exploring edge
            color[u] = 2

    dfs(entry)
    return back_edges


def is_reducible(cfg, entry):
    """
    Determine whether a CFG is reducible.

    Parameters:
    cfg(dict): mapping {node: [successors]}
    entry(str): starting node

    Returns: True if the CFG is reducible or False if the CFG is irreducible
    """
    # if 1 predecesor: merge the blocks
    # check for self edges and delete those

    def pred_count(cfg):
        # build reversed dictionary
        preds = defaultdict(list)
        for node, succs in cfg.items():
            for s in succs:
                preds[s].append(node)
        return preds

    preds_dict = pred_count(cfg)
    has_one_pred = any([len(preds_dict[x]) == 1 for x in cfg.keys()])
    self_loops = len([n for n, succs in cfg.items() if n in succs]) > 0

    return has_one_pred or self_loops


def mycfg():
    prog = json.load(sys.stdin)
    for func in prog["functions"]:
        name2block = block_map(form_blocks(func["instrs"]))
        cfg = get_cfg(name2block)
        print("digraph {} {{".format(func["name"]))

        for name in name2block:
            print("    {};".format(name))

        for name, succs in cfg.items():
            for succ in succs:
                print("    {} -> {};".format(name, succ))
        print("}")


if __name__ == "__main__":
    mycfg()
