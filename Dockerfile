FROM ubuntu:20.04
COPY bin/seq_generator /usr/local/bin/
COPY bin/libserver.a /usr/local/bin/
COPY bin/libseq_storage.so /usr/local/bin/
ENV LD_LIBRARY_PATH=/usr/local/bin
WORKDIR /usr/local/bin/
EXPOSE 8085
CMD ./seq_generator 8085