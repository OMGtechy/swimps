# See here for image contents: https://github.com/microsoft/vscode-dev-containers/tree/v0.166.1/containers/cpp/.devcontainer/base.Dockerfile

# [Choice] Debian / Ubuntu version: debian-10, debian-9, ubuntu-20.04, ubuntu-18.04
ARG VARIANT="ubuntu-20-04"
FROM mcr.microsoft.com/vscode/devcontainers/cpp:0-${VARIANT}

RUN apt-get update
RUN apt-get install -y --no-install-recommends software-properties-common=0.99.9.8
RUN add-apt-repository ppa:ubuntu-toolchain-r/test
RUN apt-get update
RUN apt-get install -y --no-install-recommends cmake=3.16.3-1ubuntu1 gcc-10=10.3.0-1ubuntu1~20.04 g++-10=10.3.0-1ubuntu1~20.04 clang-11=1:11.0.0-2~ubuntu20.04.1 libdwarf-dev=20200114-1 libunwind-dev=1.2.1-9build1 libelf-dev=0.176-1.1build1
