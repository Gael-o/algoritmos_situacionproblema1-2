import sys, json, random, subprocess, itertools, math, time
from collections import deque
def run(mode, inp):
    p = subprocess.run([EJECUTABLE], input=json.dumps({"mode":mode,"input":inp}), capture_output=True, text=True)
    return json.loads(p.stdout)
EJECUTABLE = sys.argv[1] if len(sys.argv) > 1 else "./sentinel"
random.seed(7)
bad = 0
# palindrome / lcs / kmp
def brute_pal(s):
    best=(0,0)
    for i in range(len(s)):
        for j in range(i,len(s)):
            t=s[i:j+1]
            if t==t[::-1] and j-i+1>best[1]-best[0]+1 if best!=(0,0) else t==t[::-1]:
                best=(i+1,j+1)
    return list(best)
for _ in range(300):
    a=''.join(random.choice("0123456789abcdef"[:random.randint(2,6)]) for _ in range(random.randint(1,25)))
    b=''.join(random.choice("0123456789abcdef"[:random.randint(2,6)]) for _ in range(random.randint(1,25)))
    r=run("palindrome",{"transmissions":[a,b]})["ranges"]
    if r!=[brute_pal(a),brute_pal(b)]:
        bad+=1; print("PAL",a,b,r,[brute_pal(a),brute_pal(b)])
    # lcs brute leftmost-longest in a
    best=(0,0)
    for i in range(len(a)):
        for j in range(i+1,len(a)+1):
            if a[i:j] in b and j-i>best[1]-best[0]:
                best=(i,j)
    exp=[best[0]+1,best[1]] if best[1]>0 else [0,0]
    r=run("commonsubstr",{"transmissions":[a,b]})["range"]
    if r!=exp: bad+=1; print("LCS",a,b,r,exp)
    sigs=[''.join(random.choice("ab") for _ in range(random.randint(1,4))) for _ in range(3)]
    ex=[]
    for t in (a,b):
        for s in sigs:
            p=t.find(s)+1
            ex.append({"found":True,"position":p} if p>0 else {"found":False})
    r=run("matching",{"signatures":sigs,"transmissions":[a,b]})["positions"]
    if r!=ex: bad+=1; print("KMP",sigs,a,b,r,ex)
print("strings bad:",bad)
# TSP exact vs brute
bad=0
for _ in range(60):
    n=random.randint(2,8)
    pts=[(random.randint(0,50),random.randint(0,50)) for _ in range(n)]
    M=[[round(math.dist(p,q),6) for q in pts] for p in pts]
    r=run("tsp",{"matrix":M})
    best=min(sum(M[x][y] for x,y in zip((0,)+perm,perm+(0,))) for perm in itertools.permutations(range(1,n))) if n>1 else 0
    if abs(r["cost"]-best)>1e-6 or len(r["route"])!=n+1 or r["route"][0]!=r["route"][-1] or sorted(r["route"][:-1])!=list(range(n)):
        bad+=1; print("TSP",n,r,best)
print("tsp bad:",bad)
# heuristic quality n=14 (exact, 14<=16) vs n=25 NN+2opt valid
pts=[(random.random()*100,random.random()*100) for _ in range(300)]
M=[[math.dist(p,q) for q in pts] for p in pts]
t=time.time(); r=run("tsp",{"matrix":M}); print("tsp300 cost",r["cost"],"valid",sorted(r["route"][:-1])==list(range(300)),len(r["route"]),round(time.time()-t,2),"s")
# flow vs python EK
def ek(n,edges,s,t):
    cap=[[0]*n for _ in range(n)]
    for u,v,c in edges: cap[u][v]+=c
    flow=0
    while True:
        par=[-1]*n; par[s]=s; q=deque([s])
        while q and par[t]<0:
            u=q.popleft()
            for v in range(n):
                if par[v]<0 and cap[u][v]>0: par[v]=u; q.append(v)
        if par[t]<0: return flow
        f=10**9; v=t
        while v!=s: f=min(f,cap[par[v]][v]); v=par[v]
        v=t
        while v!=s: cap[par[v]][v]-=f; cap[v][par[v]]+=f; v=par[v]
        flow+=f
def flow_detail_ok(n,edges,s,t,r,directed):
    # capacidad respetada, conservacion en cada nodo, corte = flujo maximo y el corte separa s de t
    bal=[0]*n
    for (u,v,f,c),e in zip(r["flows"],edges):
        if (u,v,c)!=(e[0],e[1],e[2]): return False
        if f>c+1e-9 or (directed and f< -1e-9) or (not directed and f< -c-1e-9): return False
        bal[u]-=f; bal[v]+=f
    if any(abs(bal[x])>1e-6 for x in range(n) if x not in (s,t)): return False
    if abs(bal[t]-r["maxFlow"])>1e-6 or abs(r["minCutCapacity"]-r["maxFlow"])>1e-6: return False
    cut={e[3] for e in r["minCut"]}
    adj=[[] for _ in range(n)]
    for i,(u,v,c) in enumerate(edges):
        if i in cut or c<=0: continue
        adj[u].append(v)
        if not directed: adj[v].append(u)
    seen={s}; q=deque([s])
    while q:
        x=q.popleft()
        for y in adj[x]:
            if y not in seen: seen.add(y); q.append(y)
    return t not in seen or r["maxFlow"]==0
bad=0
for _ in range(200):
    n=random.randint(2,12)
    edges=[(random.randrange(n),random.randrange(n),random.randint(1,20)) for _ in range(random.randint(1,40))]
    edges=[e for e in edges if e[0]!=e[1]]
    if not edges: continue
    s,t=0,n-1
    r=run("maxflow",{"n":n,"edges":[list(e) for e in edges],"source":s,"sink":t})
    if "maxFlow" not in r or r["maxFlow"]!=ek(n,edges,s,t): bad+=1; print("FLOW",n,edges,r,ek(n,edges,s,t))
    elif not flow_detail_ok(n,edges,s,t,r,True): bad+=1; print("FLOW DETAIL",n,edges,r)
    both=[(u,v,c) for (u,v,c) in edges]+[(v,u,c) for (u,v,c) in edges]
    ru=run("maxflow",{"n":n,"edges":[list(e) for e in edges],"directed":False,"source":s,"sink":t})
    if ru["maxFlow"]!=ek(n,both,s,t) or not flow_detail_ok(n,edges,s,t,ru,False): bad+=1; print("FLOW UNDIRECTED",n,edges,ru)
print("flow bad:",bad)
# propagation (caminos minimos con fallas) vs Floyd-Warshall en Python
bad=0
for _ in range(300):
    n=random.randint(1,14)
    edges=[[random.randrange(n),random.randrange(n),random.randint(1,9)] for _ in range(random.randint(0,3*n))]
    edges=[e for e in edges if e[0]!=e[1]]
    delays=[random.randint(1,30) for _ in edges]
    hop=random.choice([0,1,2.5])
    src=random.randrange(n)
    fl=[i for i in range(len(edges)) if random.random()<0.2]
    fn=[x for x in range(n) if x!=src and random.random()<0.15]
    algo=random.choice(["dijkstra","bellmanford"])
    met=random.choice(["weight","hops","time"])
    r=run("propagation",{"n":n,"edges":edges,"delays":delays,"hopDelay":hop,"source":src,"failedLinks":fl,"failedNodes":fn,"algorithm":algo,"metric":met})
    peso=lambda i:{"weight":edges[i][2],"hops":1,"time":delays[i]+hop}[met]
    INF=float("inf"); D=[[0 if i==j else INF for j in range(n)] for i in range(n)]
    usable=lambda i:i not in fl and edges[i][0] not in fn and edges[i][1] not in fn
    for i,(u,v,w) in enumerate(edges):
        if usable(i): D[u][v]=min(D[u][v],peso(i)); D[v][u]=min(D[v][u],peso(i))
    for k in range(n):
        for i in range(n):
            for j in range(n):
                if D[i][k]+D[k][j]<D[i][j]: D[i][j]=D[i][k]+D[k][j]
    ok="nodes" in r
    if ok:
        nodes={x["id"]:x for x in r["nodes"]}
        for v in range(n):
            x=nodes[v]; exp=D[src][v]
            if (exp<INF)!=x["reachable"] or (exp<INF and abs(x["distance"]-exp)>1e-9): ok=False
            if x["reachable"] and v!=src:
                p,e=x["parent"],int(x["via"]); u,w,ww=edges[e][0],edges[e][1],peso(int(x["via"]))
                if not usable(e) or {u,w}!={p,v}: ok=False
                elif abs(x["distance"]-nodes[p]["distance"]-ww)>1e-9 or x["hops"]!=nodes[p]["hops"]+1: ok=False
                elif abs(x["time"]-nodes[p]["time"]-delays[e]-hop)>1e-9: ok=False
        if [e["node"] for e in r["events"]]!=sorted([v for v in range(n) if nodes[v]["reachable"]],key=lambda v:(nodes[v]["time"],v)): ok=False
    if not ok: bad+=1; print("PROP",algo,n,edges,src,fl,fn,r)
print("propagation bad:",bad)
# voronoi: areas sum and nearest check
bad=0
for _ in range(30):
    n=random.randint(1,25)
    sites=[[random.uniform(0,100),random.uniform(0,100)] for _ in range(n)]
    r=run("voronoi",{"sites":sites,"bounds":[0,0,100,100],"queries":[[random.uniform(0,100),random.uniform(0,100)] for _ in range(5)]})
    tot=sum(c["area"] for c in r["regions"])
    if len(r["cells"])!=n or abs(tot-10000)>1e-6: bad+=1; print("VOR",n,tot)
print("voronoi bad:",bad)
