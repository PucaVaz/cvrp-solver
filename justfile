default: build

build:
    cd src && make -s

clean:
    cd src && make clean

format:
    find src -name '*.cpp' -o -name '*.h' | xargs clang-format -i

lint:
    find src -name '*.cpp' -o -name '*.h' | xargs clang-tidy -p . --quiet || true

run INSTANCE="instances/instancias_teste/n14_q30.txt" METHOD="nearest":
    ./bin/apa_jpbike --instance {{INSTANCE}} --constructive {{METHOD}}
