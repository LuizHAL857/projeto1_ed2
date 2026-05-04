CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -Werror -Werror=implicit-function-declaration -fstack-protector-all -I../include -I../unity

SRC_DIR = .
TEST_DIR = ../test
UNITY_DIR = ../unity
BUILD_DIR = ../build

APP = ./ted

APP_SRCS = $(SRC_DIR)/main.c \
	$(SRC_DIR)/trata_argumentos.c \
	$(SRC_DIR)/trata_geo.c \
	$(SRC_DIR)/trata_pm.c \
	$(SRC_DIR)/trata_qry.c \
	$(SRC_DIR)/hash_extensivel.c \
	$(SRC_DIR)/quadra.c \
	$(SRC_DIR)/habitante.c \
	$(SRC_DIR)/lista.c \
	$(SRC_DIR)/leitor_arquivos.c

APP_OBJS = $(SRC_DIR)/main.o \
	$(SRC_DIR)/trata_argumentos.o \
	$(SRC_DIR)/trata_geo.o \
	$(SRC_DIR)/trata_pm.o \
	$(SRC_DIR)/trata_qry.o \
	$(SRC_DIR)/hash_extensivel.o \
	$(SRC_DIR)/quadra.o \
	$(SRC_DIR)/habitante.o \
	$(SRC_DIR)/lista.o \
	$(SRC_DIR)/leitor_arquivos.o

TEST_HASH_BIN = $(BUILD_DIR)/test_hash_extensivel
TEST_QUADRA_BIN = $(BUILD_DIR)/test_quadra
TEST_HABITANTE_BIN = $(BUILD_DIR)/test_habitante
TEST_TRATA_BIN = $(BUILD_DIR)/test_trata_argumentos
TEST_TRATA_GEO_BIN = $(BUILD_DIR)/test_trata_geo
TEST_TRATA_PM_BIN = $(BUILD_DIR)/test_trata_pm
TEST_TRATA_QRY_BIN = $(BUILD_DIR)/test_trata_qry
TEST_LISTA_BIN = $(BUILD_DIR)/test_lista
TEST_LEITOR_BIN = $(BUILD_DIR)/test_leitor_arquivos

.PHONY: all test run-tests clean

all: $(APP)

$(APP): $(APP_OBJS)
	$(CC) $(APP_OBJS) -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(SRC_DIR)/main.o: $(SRC_DIR)/main.c
	$(CC) $(CFLAGS) -c $< -o $@

$(SRC_DIR)/trata_argumentos.o: $(SRC_DIR)/trata_argumentos.c ../include/trata_argumentos.h
	$(CC) $(CFLAGS) -c $< -o $@

$(SRC_DIR)/trata_geo.o: $(SRC_DIR)/trata_geo.c ../include/trata_geo.h ../include/hash_extensivel.h ../include/leitor_arquivos.h ../include/quadra.h ../include/lista.h
	$(CC) $(CFLAGS) -c $< -o $@

$(SRC_DIR)/trata_pm.o: $(SRC_DIR)/trata_pm.c ../include/trata_pm.h ../include/trata_geo.h ../include/habitante.h ../include/hash_extensivel.h ../include/leitor_arquivos.h ../include/lista.h ../include/quadra.h
	$(CC) $(CFLAGS) -c $< -o $@

$(SRC_DIR)/trata_qry.o: $(SRC_DIR)/trata_qry.c ../include/trata_qry.h ../include/trata_pm.h ../include/trata_geo.h ../include/leitor_arquivos.h ../include/lista.h ../include/quadra.h ../include/habitante.h
	$(CC) $(CFLAGS) -c $< -o $@

$(SRC_DIR)/hash_extensivel.o: $(SRC_DIR)/hash_extensivel.c ../include/hash_extensivel.h
	$(CC) $(CFLAGS) -c $< -o $@

$(SRC_DIR)/quadra.o: $(SRC_DIR)/quadra.c ../include/quadra.h
	$(CC) $(CFLAGS) -c $< -o $@

$(SRC_DIR)/habitante.o: $(SRC_DIR)/habitante.c ../include/habitante.h
	$(CC) $(CFLAGS) -c $< -o $@

$(SRC_DIR)/lista.o: $(SRC_DIR)/lista.c ../include/lista.h
	$(CC) $(CFLAGS) -c $< -o $@

$(SRC_DIR)/leitor_arquivos.o: $(SRC_DIR)/leitor_arquivos.c ../include/leitor_arquivos.h ../include/lista.h
	$(CC) $(CFLAGS) -c $< -o $@

$(TEST_HASH_BIN): $(SRC_DIR)/hash_extensivel.c $(TEST_DIR)/test_hash_extensivel.c $(UNITY_DIR)/unity.c ../include/hash_extensivel.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SRC_DIR)/hash_extensivel.c $(TEST_DIR)/test_hash_extensivel.c $(UNITY_DIR)/unity.c -o $@

$(TEST_QUADRA_BIN): $(SRC_DIR)/quadra.c $(SRC_DIR)/hash_extensivel.c $(TEST_DIR)/test_quadra.c $(UNITY_DIR)/unity.c ../include/quadra.h ../include/hash_extensivel.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SRC_DIR)/quadra.c $(SRC_DIR)/hash_extensivel.c $(TEST_DIR)/test_quadra.c $(UNITY_DIR)/unity.c -o $@

$(TEST_HABITANTE_BIN): $(SRC_DIR)/habitante.c $(SRC_DIR)/hash_extensivel.c $(TEST_DIR)/test_habitante.c $(UNITY_DIR)/unity.c ../include/habitante.h ../include/hash_extensivel.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SRC_DIR)/habitante.c $(SRC_DIR)/hash_extensivel.c $(TEST_DIR)/test_habitante.c $(UNITY_DIR)/unity.c -o $@

$(TEST_TRATA_BIN): $(SRC_DIR)/trata_argumentos.c $(TEST_DIR)/test_trata_argumentos.c $(UNITY_DIR)/unity.c ../include/trata_argumentos.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SRC_DIR)/trata_argumentos.c $(TEST_DIR)/test_trata_argumentos.c $(UNITY_DIR)/unity.c -o $@

$(TEST_TRATA_GEO_BIN): $(SRC_DIR)/trata_geo.c $(SRC_DIR)/leitor_arquivos.c $(SRC_DIR)/lista.c $(SRC_DIR)/quadra.c $(SRC_DIR)/hash_extensivel.c $(TEST_DIR)/test_trata_geo.c $(UNITY_DIR)/unity.c ../include/trata_geo.h ../include/leitor_arquivos.h ../include/lista.h ../include/quadra.h ../include/hash_extensivel.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SRC_DIR)/trata_geo.c $(SRC_DIR)/leitor_arquivos.c $(SRC_DIR)/lista.c $(SRC_DIR)/quadra.c $(SRC_DIR)/hash_extensivel.c $(TEST_DIR)/test_trata_geo.c $(UNITY_DIR)/unity.c -o $@

$(TEST_TRATA_PM_BIN): $(SRC_DIR)/trata_pm.c $(SRC_DIR)/trata_geo.c $(SRC_DIR)/leitor_arquivos.c $(SRC_DIR)/lista.c $(SRC_DIR)/quadra.c $(SRC_DIR)/habitante.c $(SRC_DIR)/hash_extensivel.c $(TEST_DIR)/test_trata_pm.c $(UNITY_DIR)/unity.c ../include/trata_pm.h ../include/trata_geo.h ../include/leitor_arquivos.h ../include/lista.h ../include/quadra.h ../include/habitante.h ../include/hash_extensivel.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SRC_DIR)/trata_pm.c $(SRC_DIR)/trata_geo.c $(SRC_DIR)/leitor_arquivos.c $(SRC_DIR)/lista.c $(SRC_DIR)/quadra.c $(SRC_DIR)/habitante.c $(SRC_DIR)/hash_extensivel.c $(TEST_DIR)/test_trata_pm.c $(UNITY_DIR)/unity.c -o $@

$(TEST_TRATA_QRY_BIN): $(SRC_DIR)/trata_qry.c $(SRC_DIR)/trata_pm.c $(SRC_DIR)/trata_geo.c $(SRC_DIR)/leitor_arquivos.c $(SRC_DIR)/lista.c $(SRC_DIR)/quadra.c $(SRC_DIR)/habitante.c $(SRC_DIR)/hash_extensivel.c $(TEST_DIR)/test_trata_qry.c $(UNITY_DIR)/unity.c ../include/trata_qry.h ../include/trata_pm.h ../include/trata_geo.h ../include/leitor_arquivos.h ../include/lista.h ../include/quadra.h ../include/habitante.h ../include/hash_extensivel.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SRC_DIR)/trata_qry.c $(SRC_DIR)/trata_pm.c $(SRC_DIR)/trata_geo.c $(SRC_DIR)/leitor_arquivos.c $(SRC_DIR)/lista.c $(SRC_DIR)/quadra.c $(SRC_DIR)/habitante.c $(SRC_DIR)/hash_extensivel.c $(TEST_DIR)/test_trata_qry.c $(UNITY_DIR)/unity.c -o $@

$(TEST_LISTA_BIN): $(SRC_DIR)/lista.c $(TEST_DIR)/test_lista.c $(UNITY_DIR)/unity.c ../include/lista.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SRC_DIR)/lista.c $(TEST_DIR)/test_lista.c $(UNITY_DIR)/unity.c -o $@

$(TEST_LEITOR_BIN): $(SRC_DIR)/leitor_arquivos.c $(SRC_DIR)/lista.c $(TEST_DIR)/test_leitor_arquivos.c $(UNITY_DIR)/unity.c ../include/leitor_arquivos.h ../include/lista.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SRC_DIR)/leitor_arquivos.c $(SRC_DIR)/lista.c $(TEST_DIR)/test_leitor_arquivos.c $(UNITY_DIR)/unity.c -o $@

test: $(TEST_HASH_BIN) $(TEST_QUADRA_BIN) $(TEST_HABITANTE_BIN) $(TEST_TRATA_BIN) $(TEST_TRATA_GEO_BIN) $(TEST_TRATA_PM_BIN) $(TEST_TRATA_QRY_BIN) $(TEST_LISTA_BIN) $(TEST_LEITOR_BIN)

run-tests: test
	./$(TEST_HASH_BIN)
	./$(TEST_QUADRA_BIN)
	./$(TEST_HABITANTE_BIN)
	./$(TEST_TRATA_BIN)
	./$(TEST_TRATA_GEO_BIN)
	./$(TEST_TRATA_PM_BIN)
	./$(TEST_TRATA_QRY_BIN)
	./$(TEST_LISTA_BIN)
	./$(TEST_LEITOR_BIN)

clean:
	rm -rf $(BUILD_DIR) $(APP) $(APP_OBJS)
