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
