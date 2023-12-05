- 环境
Ubuntu 22.04.3 LTS
NVIDIA-SMI 520.61.05    Driver Version: 520.61.05    CUDA Version: 11.8 

- 效果
![image](https://github.com/swoole/phpy/assets/9689137/bef8f25a-8003-46c6-95e2-a9deb1f655c5)


- Dockefile
```
FROM nvidia/cuda:11.8.0-devel-ubuntu22.04

RUN apt-get update && \
    apt-get install -y --no-install-recommends build-essential git wget software-properties-common && \
    add-apt-repository ppa:ondrej/php && apt-get update && \
	DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends php-cli php-dev && \
	rm -rf /var/lib/apt/lists/*

RUN wget https://repo.anaconda.com/miniconda/Miniconda3-latest-Linux-x86_64.sh -O ~/miniconda.sh && \
    chmod +x ~/miniconda.sh && \
    mkdir -p /opt/conda && \
    ~/miniconda.sh -b -u -p /opt/conda && \
    rm ~/miniconda.sh && \
    /opt/conda/bin/conda init bash

ENV PATH="/opt/conda/bin:$PATH"

RUN conda install pytorch torchvision torchaudio pytorch-cuda=11.8 -c pytorch -c nvidia && \
	conda install -c huggingface transformers

RUN git clone https://github.com/swoole/phpy.git /app/phpy

WORKDIR /app/phpy
RUN sed -i 's@anaconda3@conda@' config.m4

RUN phpize && \
    ./configure && \
    make install && \
    echo "extension=phpy.so" > /etc/php/8.2/cli/conf.d/20_phpy.ini

RUN php -m | grep -i phpy

CMD [ "bash" ]
```

- 使用
```
docker run --rm -it --gpus all --name phpy phpy bash
cd examples
php pipeline.php
```

- 安装docker和cuda
```
sudo apt-get update
sudo apt-get install \
 ca-certificates \
 curl \
 gnupg \
 lsb-release -y
curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo gpg --dearmor -o /usr/share/keyrings/docker-archive-keyring.gpg
echo \
"deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/docker-archive-keyring.gpg] https://download.docker.com/linux/ubuntu \
$(lsb_release -cs) stable" | sudo tee /etc/apt/sources.list.d/docker.list > /dev/null
sudo apt-get update
sudo apt-get install docker-ce docker-ce-cli containerd.io -y

distribution=$(. /etc/os-release;echo $ID$VERSION_ID) \
&& curl -s -L https://nvidia.github.io/nvidia-docker/gpgkey | sudo apt-key add - \
&& curl -s -L https://nvidia.github.io/nvidia-docker/$distribution/nvidia-docker.list | sudo tee /etc/apt/sources.list.d/nvidia-docker.list
sudo apt-get update
sudo apt-get install -y nvidia-docker2

wget https://developer.download.nvidia.com/compute/cuda/11.8.0/local_installers/cuda_11.8.0_520.61.05_linux.run 
sudo sh ./cuda_11.8.0_520.61.05_linux.run
```
