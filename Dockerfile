# Builds and runs the SmartSuggest C++ backend (REST API + WhatsApp webhook).
FROM gcc:13 AS build
WORKDIR /app
COPY Makefile ./
COPY include ./include
COPY src ./src
RUN make app

FROM debian:bookworm-slim
WORKDIR /app
RUN apt-get update && apt-get install -y --no-install-recommends libstdc++6 && rm -rf /var/lib/apt/lists/*
COPY --from=build /app/app ./app
COPY words.txt ./words.txt
COPY public ./public
ENV PORT=8080
EXPOSE 8080
CMD ["./app", "--server"]
