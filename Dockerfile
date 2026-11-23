# Multi-stage Dockerfile for Render / Railway / Cloud Deployments
# Stage 1: Build React 18 Frontend
FROM node:20-alpine AS frontend-builder
WORKDIR /app/frontend
COPY frontend/package*.json ./
RUN npm install
COPY frontend/ ./
RUN npm run build

# Stage 2: Compile C++17 Application
FROM gcc:12 AS cpp-builder
WORKDIR /app
COPY include/ include/
COPY src/ src/
COPY Makefile words.txt ./
RUN make app

# Stage 3: Lightweight Production Runner
FROM ubuntu:22.04
WORKDIR /app
RUN apt-get update && apt-get install -y --no-install-recommends ca-certificates && rm -rf /var/lib/apt/lists/*
COPY --from=cpp-builder /app/app ./app
COPY --from=cpp-builder /app/words.txt ./words.txt
COPY --from=frontend-builder /app/public ./public

# Render dynamically injects $PORT (default 8080)
ENV PORT=8080
EXPOSE 8080

CMD ["./app", "--server"]
