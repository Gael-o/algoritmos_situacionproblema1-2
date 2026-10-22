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
bad=0
for _ in range(200):
    n=random.randint(2,12)
    edges=[(random.randrange(n),random.randrange(n),random.randint(1,20)) for _ in range(random.randint(1,40))]
    edges=[e for e in edges if e[0]!=e[1]]
    if not edges: continue
    s,t=0,n-1
    r=run("maxflow",{"n":n,"edges":[list(e) for e in edges],"source":s,"sink":t})
    if "maxFlow" not in r or r["maxFlow"]!=ek(n,edges,s,t): bad+=1; print("FLOW",n,edges,r,ek(n,edges,s,t))
print("flow bad:",bad)
# voronoi: areas sum and nearest check
bad=0
for _ in range(30):
    n=random.randint(1,25)
    sites=[[random.uniform(0,100),random.uniform(0,100)] for _ in range(n)]
    r=run("voronoi",{"sites":sites,"bounds":[0,0,100,100],"queries":[[random.uniform(0,100),random.uniform(0,100)] for _ in range(5)]})
    tot=sum(c["area"] for c in r["regions"])
    if len(r["cells"])!=n or abs(tot-10000)>1e-6: bad+=1; print("VOR",n,tot)
print("voronoi bad:",bad)
