# C++ Microservices with Kubernetes

A hands-on learning project demonstrating microservices architecture patterns and Kubernetes orchestration using C++. This project implements a simple data processing pipeline with three loosely-coupled services.

## Table of Contents

- [Project Overview](#project-overview)
- [Architecture](#architecture)
- [Key Concepts](#key-concepts)
- [Prerequisites](#prerequisites)
- [Quick Start](#quick-start)
- [Building the Services](#building-the-services)
- [Deploying to Kubernetes](#deploying-to-kubernetes)
- [Testing the Application](#testing-the-application)
- [Troubleshooting](#troubleshooting)
- [Project Structure](#project-structure)
- [Learning Outcomes](#learning-outcomes)

---

## Project Overview

This project demonstrates a **microservices architecture** where three independent services communicate over HTTP to form a data processing pipeline:

1. **Producer** - Generates random data
2. **Processor** - Fetches data from Producer and transforms it
3. **Consumer** - Polls Processor and displays results

Each service is:
- Written in C++ using [cpp-httplib](https://github.com/yhirose/cpp-httplib) for HTTP
- Containerized with Docker
- Deployed and orchestrated using Kubernetes
- Configured via environment variables and ConfigMaps

---

## Architecture

### Service Communication Flow

```
┌──────────────┐
│   Consumer   │ (Port 8082, NodePort - External Access)
│              │
│ • Polls every│
│   5 seconds  │
│ • HTTP server│
└──────┬───────┘
       │
       │ GET /process
       ▼
┌──────────────┐
│  Processor   │ (Port 8081, ClusterIP)
│              │
│ • Multiplies │
│   value by 2 │
└──────┬───────┘
       │
       │ GET /data
       ▼
┌──────────────┐
│   Producer   │ (Port 8080, ClusterIP)
│              │
│ • Generates  │
│   random 1-100│
└──────────────┘
```

### Data Flow

1. **Consumer** periodically calls **Processor** at `http://processor:8081/process`
2. **Processor** calls **Producer** at `http://producer:8080/data`
3. **Producer** returns: `{"value": 42}`
4. **Processor** transforms and returns: `{"original": 42, "processed": 84}`
5. **Consumer** logs: `[CONSUME] Original: 42, Processed: 84`

### Kubernetes Resources

```
┌─────────────────────────────────────────────────────┐
│                    Kubernetes Cluster                │
│                                                       │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐  │
│  │  ConfigMap  │  │  ConfigMap  │  │  ConfigMap  │  │
│  │  producer   │  │  processor  │  │  consumer   │  │
│  └──────┬──────┘  └──────┬──────┘  └──────┬──────┘  │
│         │                │                │         │
│         ▼                ▼                ▼         │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐  │
│  │ Deployment  │  │ Deployment  │  │ Deployment  │  │
│  │ (1 replica) │  │ (1 replica) │  │ (1 replica) │  │
│  └──────┬──────┘  └──────┬──────┘  └──────┬──────┘  │
│         │                │                │         │
│         ▼                ▼                ▼         │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐  │
│  │   Service   │  │   Service   │  │   Service   │  │
│  │  ClusterIP  │  │  ClusterIP  │  │  NodePort   │  │
│  │  :8080      │  │  :8081      │  │  :8082      │  │
│  └─────────────┘  └─────────────┘  └─────────────┘  │
│                                           │         │
└───────────────────────────────────────────┼─────────┘
                                            │
                                            ▼
                                     External Access
```

---

## Key Concepts

### Microservices Patterns

- **Service Independence**: Each service runs in its own container with isolated dependencies
- **HTTP-based Communication**: RESTful APIs for inter-service communication
- **Service Discovery**: Kubernetes DNS resolves service names (e.g., `http://producer:8080`)
- **Configuration Externalization**: Environment variables via ConfigMaps
- **Polling Pattern**: Consumer uses periodic polling (alternative to event-driven)

### Kubernetes Resources Used

| Resource | Purpose | Example |
|----------|---------|---------|
| **Deployment** | Manages pod lifecycle, ensures desired replica count | `producer`, `processor`, `consumer` deployments |
| **Service** | Stable network endpoint for pod discovery | `producer.default.svc.cluster.local` |
| **ConfigMap** | Stores non-sensitive configuration | Port numbers, service URLs, poll intervals |
| **ClusterIP** | Internal-only service (default) | Producer & Processor are not exposed externally |
| **NodePort** | Exposes service on node's IP | Consumer accessible for testing at `localhost:30xxx` |

### C++ Libraries

- **[cpp-httplib](https://github.com/yhirose/cpp-httplib)**: Header-only HTTP server/client library
- **[nlohmann/json](https://github.com/nlohmann/json)**: Modern C++ JSON library
- **Standard Library**: `<thread>`, `<chrono>` for Consumer's background polling

---

## Prerequisites

### Required Tools

- **Docker**: For building container images
- **Kubernetes**: One of the following:
  - [Docker Desktop](https://www.docker.com/products/docker-desktop) (with Kubernetes enabled)
  - [Minikube](https://minikube.sigs.k8s.io/docs/)
  - [kind](https://kind.sigs.k8s.io/) (Kubernetes in Docker)
- **kubectl**: Kubernetes command-line tool
- **make**: For building C++ applications
- **g++**: C++17 compatible compiler

### Verify Installation

```bash
# Check Docker
docker --version

# Check Kubernetes cluster is running
kubectl cluster-info

# Check kubectl can connect
kubectl get nodes
```

---

## Quick Start

### 1. Clone and Navigate

```bash
cd /home/bean2778/cpp-k8-microservices
```

### 2. Build Docker Images

The Dockerfiles expect to be built from the **project root** (to access shared headers).

```bash
# Build Producer
docker build -t cpp-producer:latest -f producer/Dockerfile .

# Build Processor
docker build -t cpp-processor:latest -f processor/Dockerfile .

# Build Consumer
docker build -t cpp-consumer:latest -f consumer/Dockerfile .
```

### 3. Load Images into Kubernetes

**For Minikube:**
```bash
minikube image load cpp-producer:latest
minikube image load cpp-processor:latest
minikube image load cpp-consumer:latest
```

**For kind:**
```bash
kind load docker-image cpp-producer:latest
kind load docker-image cpp-processor:latest
kind load docker-image cpp-consumer:latest
```

**For Docker Desktop:** Images are automatically available (skip this step).

### 4. Deploy to Kubernetes

```bash
# Apply ConfigMaps
kubectl apply -f k8s/configmap.yaml

# Deploy services
kubectl apply -f k8s/producer.yaml
kubectl apply -f k8s/processor.yaml
kubectl apply -f k8s/consumer.yaml
```

### 5. Verify Deployment

```bash
# Check all pods are running
kubectl get pods

# You should see:
# NAME                         READY   STATUS    RESTARTS   AGE
# producer-xxxxxxxxxx-xxxxx    1/1     Running   0          30s
# processor-xxxxxxxxxx-xxxxx   1/1     Running   0          30s
# consumer-xxxxxxxxxx-xxxxx    1/1     Running   0          30s
```

### 6. View Consumer Logs

```bash
kubectl logs -f deployment/consumer

# You should see output like:
# [CONSUME] Original: 42, Processed: 84
# [CONSUME] Original: 73, Processed: 146
```

---

## Building the Services

### Local Development Build

Each service can be built locally for testing:

```bash
# Producer
cd producer
make
./producer

# Processor
cd processor
make
./processor

# Consumer
cd consumer
make
./consumer
```

Note: For local testing without Kubernetes, you'll need to:
- Run each service in a separate terminal
- Update environment variables to use `localhost` instead of service names
- Ensure ports don't conflict

### Docker Build Details

The Dockerfiles use a multi-step process:

1. Start with `gcc:11` base image
2. Copy shared headers (`httplib.h`, `json.hpp`) from project root
3. Copy service-specific source and Makefile
4. Compile the C++ application
5. Expose the appropriate port
6. Run the compiled binary

Example structure in [producer/Dockerfile](producer/Dockerfile):
```dockerfile
FROM gcc:11
WORKDIR /app
COPY httplib.h json.hpp /app/
COPY producer/producer.cpp producer/Makefile /app/
RUN make
EXPOSE 8080
CMD ["./producer"]
```

---

## Deploying to Kubernetes

### Understanding the Manifests

#### ConfigMaps ([k8s/configmap.yaml](k8s/configmap.yaml))

Stores environment-specific configuration:

```yaml
apiVersion: v1
kind: ConfigMap
metadata:
  name: consumer-config
data:
  PORT: "8082"
  PROCESSOR_HOST: "processor"  # Kubernetes service name
  PROCESSOR_PORT: "8081"
  POLL_INTERVAL_SECONDS: "5"
```

#### Deployments

Each deployment specifies:
- **Replicas**: Currently set to 1 (can scale horizontally)
- **Image Pull Policy**: `Never` (uses local images)
- **Resources**: Memory/CPU requests and limits
- **Environment**: Injected from ConfigMap via `envFrom`

Example from [k8s/producer.yaml](k8s/producer.yaml):
```yaml
spec:
  replicas: 1
  containers:
  - name: producer
    image: cpp-producer:latest
    imagePullPolicy: Never
    envFrom:
    - configMapRef:
        name: producer-config
    resources:
      requests:
        memory: "64Mi"
        cpu: "100m"
```

#### Services

- **Producer/Processor**: `ClusterIP` (internal only, accessible via DNS like `http://producer:8080`)
- **Consumer**: `NodePort` (exposes random port 30000-32767 for external access)

### Deployment Commands

```bash
# Deploy all resources
kubectl apply -f k8s/

# Or deploy individually
kubectl apply -f k8s/configmap.yaml
kubectl apply -f k8s/producer.yaml
kubectl apply -f k8s/processor.yaml
kubectl apply -f k8s/consumer.yaml

# Check deployment status
kubectl get deployments
kubectl get pods
kubectl get services

# View detailed pod info
kubectl describe pod <pod-name>
```

### Scaling Services

```bash
# Scale producer to 2 replicas
kubectl scale deployment producer --replicas=2

# Verify
kubectl get pods -l app=producer
```

Note: The current implementation uses hardcoded URLs in Processor ([processor.cpp:27](processor/processor.cpp#L27)), so scaling may require code updates to use environment variables consistently.

---

## Testing the Application

### 1. Check Pod Status

```bash
kubectl get pods

# All should show STATUS: Running
```

### 2. View Consumer Output

The consumer automatically polls every 5 seconds:

```bash
kubectl logs -f deployment/consumer

# Expected output:
# Consumer starting...
# Consumer configuration:
#   Port: 8082
#   Processor URL: http://processor:8081
#   Poll interval: 5s
# [CONSUME] Original: 67, Processed: 134
# [CONSUME] Original: 23, Processed: 46
```

### 3. Manual Testing via Consumer Endpoints

Get the NodePort assigned to the consumer:

```bash
kubectl get service consumer

# Example output:
# NAME       TYPE       PORT(S)          AGE
# consumer   NodePort   8082:31234/TCP   5m
```

The consumer service is accessible at `localhost:31234` (your port may differ).

#### Health Check

```bash
curl http://localhost:31234/health

# Response:
# {"service":"consumer","status":"healthy"}
```

#### Manual Consume Trigger

```bash
curl http://localhost:31234/consume

# Response:
# {"original":89,"processed":178}
```

### 4. Test Producer Directly

Port-forward to access internal services:

```bash
# Forward producer port
kubectl port-forward service/producer 8080:8080

# In another terminal:
curl http://localhost:8080/data

# Response:
# {"value":42}
```

### 5. Test Processor Directly

```bash
# Forward processor port
kubectl port-forward service/processor 8081:8081

# In another terminal:
curl http://localhost:8081/process

# Response:
# {"original":58,"processed":116}
```

### 6. View All Logs

```bash
# Producer logs
kubectl logs -f deployment/producer

# Processor logs
kubectl logs -f deployment/processor

# Consumer logs
kubectl logs -f deployment/consumer
```

---

## Troubleshooting

### Pods Not Starting

**Check pod status:**
```bash
kubectl get pods
kubectl describe pod <pod-name>
```

**Common issues:**
- **ImagePullBackOff**: Image not loaded into cluster
  - Solution: Re-run `minikube image load` or `kind load docker-image`
- **CrashLoopBackOff**: Application crashes on startup
  - Solution: Check logs with `kubectl logs <pod-name>`

### Services Can't Communicate

**Verify service DNS resolution:**
```bash
# Exec into consumer pod
kubectl exec -it deployment/consumer -- sh

# Test DNS resolution
nslookup processor
nslookup producer

# Test connectivity
apt-get update && apt-get install -y curl
curl http://producer:8080/data
```

**Common issues:**
- Service names must match DNS names (e.g., `producer`, not `producer-service`)
- Services must be in the same namespace (default: `default`)
- Ports must match service and container configurations

### Consumer Not Showing Output

**Check if background thread is running:**
```bash
kubectl logs deployment/consumer | grep "Background consumption"
```

**Check processor connectivity:**
```bash
kubectl logs deployment/consumer | grep ERROR
```

**Verify configuration:**
```bash
kubectl get configmap consumer-config -o yaml
```

### Debugging Service Communication

**View processor logs to see if it receives requests:**
```bash
kubectl logs -f deployment/processor

# Should show:
# Received: 42, Processed: 84
```

**View producer logs:**
```bash
kubectl logs -f deployment/producer

# Should show:
# Generated: 42
```

### Rebuilding After Code Changes

```bash
# 1. Rebuild Docker image
docker build -t cpp-producer:latest -f producer/Dockerfile .

# 2. Reload into cluster
minikube image load cpp-producer:latest  # or kind load

# 3. Restart deployment
kubectl rollout restart deployment/producer

# 4. Verify update
kubectl rollout status deployment/producer
```

### Common Configuration Issues

| Issue | Symptom | Solution |
|-------|---------|----------|
| Hardcoded URL in [processor.cpp:27](processor/processor.cpp#L27) | Processor can't find Producer | Update to use `producerUrl` variable (defined but unused) |
| Port mismatch | Connection refused | Verify ConfigMap PORT matches container port and service targetPort |
| Wrong service name | DNS resolution fails | Ensure service name in ConfigMap matches Service metadata.name |

---

## Project Structure

```
cpp-k8-microservices/
│
├── producer/
│   ├── producer.cpp       # HTTP server generating random data
│   ├── Dockerfile         # Container build instructions
│   └── Makefile           # C++ build configuration
│
├── processor/
│   ├── processor.cpp      # HTTP server calling Producer, transforms data
│   ├── Dockerfile
│   └── Makefile
│
├── consumer/
│   ├── consumer.cpp       # Background poller + HTTP server
│   ├── Dockerfile
│   └── Makefile
│
├── k8s/
│   ├── configmap.yaml     # Environment configuration for all services
│   ├── producer.yaml      # Deployment + Service (ClusterIP)
│   ├── processor.yaml     # Deployment + Service (ClusterIP)
│   └── consumer.yaml      # Deployment + Service (NodePort)
│
├── httplib.h              # Shared HTTP library (cpp-httplib)
├── json.hpp               # Shared JSON library (nlohmann/json)
└── README.md              # This file
```

### File Descriptions

| File | Lines | Purpose |
|------|-------|---------|
| [producer/producer.cpp](producer/producer.cpp) | 39 | Simple HTTP server with `/data` endpoint generating random 1-100 |
| [processor/processor.cpp](processor/processor.cpp) | 63 | HTTP server with `/process` endpoint that calls Producer and multiplies value by 2 |
| [consumer/consumer.cpp](consumer/consumer.cpp) | 128 | Background thread polling Processor every 5s, plus HTTP server for manual testing |

---

## Learning Outcomes

### Microservices Concepts

- ✅ **Service Decomposition**: Breaking functionality into independent services
- ✅ **HTTP-based Communication**: RESTful APIs for inter-service messaging
- ✅ **Service Discovery**: Using Kubernetes DNS for dynamic service lookup
- ✅ **Externalized Configuration**: Decoupling config from code using environment variables
- ✅ **Stateless Design**: Each service can be scaled horizontally

### Kubernetes Concepts

- ✅ **Deployments**: Managing application lifecycle, rollouts, and rollbacks
- ✅ **Services**: Providing stable networking for ephemeral pods
- ✅ **ConfigMaps**: Storing configuration separate from container images
- ✅ **Service Types**: ClusterIP (internal) vs NodePort (external access)
- ✅ **Resource Management**: Setting CPU/memory requests and limits
- ✅ **Health Checks**: Using endpoints like `/health` for liveness probes (ready for extension)

### C++ in Cloud-Native Development

- ✅ **Header-only Libraries**: Using modern C++ libraries (httplib, json)
- ✅ **Containerization**: Building minimal Docker images for C++ apps
- ✅ **Environment Variables**: Runtime configuration in C++ (`getenv`)
- ✅ **Multi-threading**: Background threads for async operations (`std::thread`)

---

## Next Steps for Learning

### Enhancements to Try

1. **Add Health Checks**:
   - Implement readiness/liveness probes in Kubernetes
   - Add `/ready` endpoint to check dependencies

2. **Implement Message Queue**:
   - Replace HTTP polling with RabbitMQ or Redis pub/sub
   - Learn event-driven microservices

3. **Add Observability**:
   - Implement structured logging
   - Add Prometheus metrics endpoint
   - Visualize with Grafana

4. **Improve Resilience**:
   - Add retry logic with exponential backoff
   - Implement circuit breaker pattern
   - Add request timeouts

5. **Database Integration**:
   - Add persistence layer (PostgreSQL, MongoDB)
   - Learn StatefulSets and PersistentVolumes

6. **API Gateway**:
   - Add NGINX Ingress or Traefik
   - Implement rate limiting and authentication

7. **CI/CD Pipeline**:
   - Automate builds with GitHub Actions
   - Implement GitOps with ArgoCD

### Recommended Resources

- [Kubernetes Documentation](https://kubernetes.io/docs/)
- [Microservices Patterns](https://microservices.io/patterns/index.html) by Chris Richardson
- [12-Factor App Methodology](https://12factor.net/)
- [C++ cpp-httplib GitHub](https://github.com/yhirose/cpp-httplib)

---

## License

This is a learning project. Feel free to modify and experiment!

---

**Happy Learning!** 🚀
