DOCKER=docker
REGISTRY=docker.kbt.io

alpine:
	$(DOCKER) build -t $(REGISTRY)/geos/build-test:alpine .
	#$(DOCKER) build -t geos/build .
	#$(DOCKER) tag geos/build $(REGISTRY)/geos/build

push: alpine-push

alpine-push:
	$(DOCKER) push $(REGISTRY)/geos/build-test:alpine
