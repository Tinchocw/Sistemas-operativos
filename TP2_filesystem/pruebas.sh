#!/usr/bin/sudo bash

# Eliminar la carpeta creada
if [ -f file.fisopfs ]; then
   rm file.fisopfs

fi

# shellcheck disable=SC2034
RED='\033[0;31m'
# shellcheck disable=SC2034
GREEN='\033[0;32m'
# shellcheck disable=SC2034
BLUE='\033[0;34m'
# shellcheck disable=SC2034
NC='\033[0m' # No Color

mkdir prueba_tests
./fisopfs prueba_tests

echo "Corriendo suite sobre una carpeta vacía --> prueba_tests"

# Función para registrar fallos
log_failure() {
  local message="$1"
  echo "$message" >> error_log.txt
}

# Assertion para equidad
assert_eq() {
  local expected="$1"
  local actual="$2"
  local msg="${3-}"

  if [ "$expected" == "$actual" ]; then
    echo -e "${GREEN}OK${NC}"
    return 0
  else
    [ "${#msg}" -gt 0 ] && log_failure "$expected == $actual :: $msg" || true
    echo -e "${RED}FAIL${NC}"
    echo -e "${RED}$msg${NC}"
    echo ""
    return 1
  fi
}


echo " "
echo "---------------------------------------------------------------------------"
echo "                            Mkdir Tests                                    "
echo "---------------------------------------------------------------------------"

echo "Test 1: Create a folder with mkdir"
mkdir prueba_tests/carpeta1

assert_eq "0" "$?" "${RED}Error al crear carpeta1${NC}"
rmdir prueba_tests/carpeta1

echo""

echo "Test 2: Fail trying to create a folder that already exists"
mkdir prueba_tests/carpeta1
output=$(mkdir prueba_tests/carpeta1 2>&1)
expected="mkdir: cannot create directory ‘prueba_tests/carpeta1’: File exists"

assert_eq "$expected" "$output" "${RED}Error al crear carpeta1${NC}"
 rmdir prueba_tests/carpeta1

echo " "
echo "---------------------------------------------------------------------------"
echo "                          Remove Directory Tests                           "
echo "---------------------------------------------------------------------------"

echo "Test 3: rmdir - Remove directory efficiently "
mkdir prueba_tests/carpeta1
rmdir prueba_tests/carpeta1

assert_eq "0" "$?" "${RED}Error al eliminar carpeta1${NC}"

echo ""

echo "Test 4: rmdir - remove non-existent directory"
output=$(rmdir prueba_tests/carpeta1 2>&1)
expected="rmdir: failed to remove 'prueba_tests/carpeta1': No such file or directory"

assert_eq "$expected" "$output" "${RED}Error al eliminar carpeta1${NC}"

echo ""

echo "Test 5: rmdir - remove inefficiently "

 mkdir prueba_tests/carpeta2
 touch prueba_tests/carpeta2/archivo1

output=$(rmdir prueba_tests/carpeta2 2>&1)
expected="rmdir: failed to remove 'prueba_tests/carpeta2': Directory not empty"

rm -r prueba_tests/carpeta2/archivo1
rm -r prueba_tests/carpeta2

assert_eq "$expected" "$output" "${RED}Error al eliminar carpeta1${NC}"

echo " "
echo "---------------------------------------------------------------------------"
echo "                           Touch Tests                                     "
echo "---------------------------------------------------------------------------"

echo "Test 6: Create a file with touch"

touch prueba_tests/archivo1
output=$( ls prueba_tests)

assert_eq "archivo1" "$output"
rm prueba_tests/archivo1

echo " "
echo "---------------------------------------------------------------------------"
echo "                           Removing File Tests                               "
echo "---------------------------------------------------------------------------"
echo " "

echo "Test 7: rm file"
touch prueba_tests/archivo1
rm prueba_tests/archivo1
output=$(ls prueba_tests)

assert_eq "" "$output"
assert_eq "0" "$?" "${RED}Error al eliminar archivo1${NC}"

echo " "

echo "Test 8: rm non-existent file"
output=$(rm prueba_tests/archivo1 2>&1)
expected="rm: cannot remove 'prueba_tests/archivo1': No such file or directory"

assert_eq "$expected" "$output" "${RED}Error al eliminar archivo1${NC}"

echo " "

echo "Test 9: Try to remove a directory with rm directory"
mkdir prueba_tests/carpeta1
output=$(rm prueba_tests/carpeta1 2>&1)
expected="rm: cannot remove 'prueba_tests/carpeta1': Is a directory"
rmdir prueba_tests/carpeta1

assert_eq "$output" "$expected" "${RED}Error al eliminar carpeta1${NC}"
assert_eq "0" "$?" "${RED}Error al eliminar carpeta1${NC}"

echo " "
echo "---------------------------------------------------------------------------"
echo "                                Write Tests                                "
echo "---------------------------------------------------------------------------"

echo "Test 10: write in a file"
touch prueba_tests/archivo1
echo "Hola mundo" | (tee prueba_tests/archivo1 >/dev/null)
output=$(cat prueba_tests/archivo1)
expected="Hola mundo"
rm prueba_tests/archivo1

assert_eq "$expected" "$output" "${RED}Error al escribir en archivo1${NC}"

echo " "

echo "Test 11: write in a file appending"
touch prueba_tests/archivo1
echo "Hola" | tee prueba_tests/archivo1 > /dev/null
echo "como estas" | tee -a prueba_tests/archivo1 > /dev/null
output=$(cat prueba_tests/archivo1)
expected=$"Hola
como estas"
rm prueba_tests/archivo1

assert_eq "$expected" "$output" "${RED}Error al escribir en archivo1${NC}"

echo " "

echo "Test 12: fail to write a directory"
mkdir prueba_tests/carpeta1
output=$(echo "Hola mundo" | (tee prueba_tests/carpeta1 > /dev/null) 2>&1)
expected="tee: prueba_tests/carpeta1: Is a directory"

rmdir prueba_tests/carpeta1

assert_eq "$expected" "$output" "${RED}Error al escribir en carpeta1${NC}"


echo " "
echo "---------------------------------------------------------------------------"
echo "                                Read Tests                                 "
echo "---------------------------------------------------------------------------"

echo "Test 13: read a file"
touch prueba_tests/archivo1
echo "Hola mundo" |  tee prueba_tests/archivo1 > /dev/null
output=$(cat prueba_tests/archivo1)
expected="Hola mundo"
rm prueba_tests/archivo1

assert_eq "$expected" "$output" "${RED}Error al leer archivo1${NC}"

echo " "

echo "Test 14: read a non-existent file"
output=$(cat prueba_tests/archivo1 2>&1)
expected="cat: prueba_tests/archivo1: No such file or directory"

assert_eq "$expected" "$output" "${RED}Error al leer archivo1${NC}"

echo " "

echo "Test 15: fail to read a directory with cat"
mkdir prueba_tests/carpeta1
output=$(cat prueba_tests/carpeta1 2>&1)
expected="cat: prueba_tests/carpeta1: Is a directory"
rmdir prueba_tests/carpeta1

assert_eq "$expected" "$output" "${RED}Error al leer carpeta1${NC}"

echo " "

echo "Test 16: read a file with more than 1 line"
touch prueba_tests/archivo1
echo "Hola" |  tee prueba_tests/archivo1 > /dev/null
echo "como estas" |  tee -a prueba_tests/archivo1 > /dev/null
output=$(cat prueba_tests/archivo1)
expected=$"Hola
como estas"
rm prueba_tests/archivo1

assert_eq "$expected" "$output" "${RED}Error al leer archivo1${NC}"


echo " "
echo "---------------------------------------------------------------------------"
echo "                                Getattr Tests                              "
echo "---------------------------------------------------------------------------"
echo " "

echo "Test 17: Get the attributes of a file"
touch prueba_tests/archivo1
size=$(stat -c "%s" prueba_tests/archivo1)
blocks=$(stat -c "%b" prueba_tests/archivo1)
io_block=$(stat -c "%o" prueba_tests/archivo1)
access_time=$(stat -c "%x" prueba_tests/archivo1)
modify_time=$(stat -c "%y" prueba_tests/archivo1)
change_time=$(stat -c "%z" prueba_tests/archivo1)

expected_size="0"
expected_blocks="0"
expected_io_block="4096"
expected_access_time=${access_time}
expected_modify_time=${modify_time}
expected_change_time=${change_time}


assert_eq "$expected_size" "$size" "${RED}Error al obtener el tamaño de archivo1${NC}"
assert_eq "$expected_blocks" "$blocks" "${RED}Error al obtener el número de bloques de archivo1${NC}"
assert_eq "$expected_io_block" "$io_block" "${RED}Error al obtener el tamaño de bloque de archivo1${NC}"
assert_eq "$expected_access_time" "$access_time" "${RED}Error al obtener el tiempo de acceso de archivo1${NC}"
assert_eq "$expected_modify_time" "$modify_time" "${RED}Error al obtener el tiempo de modificación de archivo1${NC}"
assert_eq "$expected_change_time" "$change_time" "${RED}Error al obtener el tiempo de cambio de archivo1${NC}"

rm prueba_tests/archivo1

echo " "
echo "---------------------------------------------------------------------------"
echo "                                readdir Tests                              "
echo "---------------------------------------------------------------------------"
echo " "

echo "Test 18: Read the content of a directory with ls"
mkdir prueba_tests/carpeta1
touch prueba_tests/carpeta1/archivo1
touch prueba_tests/carpeta1/archivo2
output=$(ls prueba_tests/carpeta1)
expected="archivo1
archivo2"
rm prueba_tests/carpeta1/archivo1
rm prueba_tests/carpeta1/archivo2
rm -r prueba_tests/carpeta1

assert_eq "$expected" "$output" "${RED}Error al leer carpeta1${NC}"

echo " "

echo "Test 19: Read the content of a non-existent directory with ls"
output=$(ls prueba_tests/carpeta1 2>&1)
expected="ls: cannot access 'prueba_tests/carpeta1': No such file or directory"

assert_eq "$expected" "$output" "${RED}Error al leer carpeta1${NC}"

echo " "

echo "Test 20: Read the content of a file with ls"

touch prueba_tests/archivo1
output=$( ls prueba_tests/archivo1/ 2>&1)
expected="ls: cannot access 'prueba_tests/archivo1/': Not a directory"

assert_eq "$expected" "$output" "${RED}Error al leer archivo1${NC}"
rm prueba_tests/archivo1

echo " "
echo "---------------------------------------------------------------------------"
echo "                                init Tests                                 "
echo "---------------------------------------------------------------------------"
echo " "

echo "Test 21: Initialize an empty file system"

mkdir init_test

./fisopfs init_test
rm init_test/archivo1
output=$( ls init_test)

expected=""

assert_eq "$expected" "$output" "${RED}Error al inicializar el sistema de archivos${NC}"

umount init_test
rmdir init_test

echo " "
echo "---------------------------------------------------------------------------"
echo "                                utimens Tests                              "
echo "---------------------------------------------------------------------------"
echo " "

echo "Test 22: Update the timestamp of a file with utimens"

touch prueba_tests/archivo1 -d "2021-06-01 12:00:00" prueba_tests/archivo1
output=$(ls -l --time-style=full-iso prueba_tests | grep archivo1 | awk '{print $6, $7}')
expected="2021-06-01 12:00:00.000000000"
rm prueba_tests/archivo1

assert_eq "$expected" "$output" "${RED}Error al actualizar el tiempo de archivo1${NC}"

echo " "
echo "---------------------------------------------------------------------------"
echo "                                Persistency Tests                          "
echo "---------------------------------------------------------------------------"

echo "Test 23: Create a file and directory, unmount and mount again"

mkdir solucion_esperada
 ./fisopfs solucion_esperada # Mount the file system in a new folder
touch solucion_esperada/archivo1
mkdir solucion_esperada/carpeta1
expected=$( ls solucion_esperada)


umount solucion_esperada # Unmount solution
rm -r solucion_esperada


mkdir checkeo_persistencia
 ./fisopfs checkeo_persistencia # Mount the file system in a new folder
output=$( ls checkeo_persistencia)


rm -r checkeo_persistencia/archivo1
rm -r checkeo_persistencia/carpeta1
umount checkeo_persistencia
rm -r checkeo_persistencia

assert_eq "$expected" "$output" "${RED}Error al montar y desmontar${NC}"

echo " "

echo "Test 24: Create a file inside a folder, unmount and mount again"

mkdir solucion_esperada
 ./fisopfs solucion_esperada # Mount the file system in a new folder

mkdir solucion_esperada/carpeta1
touch solucion_esperada/carpeta1/archivo11
expected=$( ls solucion_esperada/carpeta1)

umount solucion_esperada # Unmount solution
rm -r solucion_esperada


mkdir checkeo_persistencia
./fisopfs checkeo_persistencia
output=$( ls checkeo_persistencia/carpeta1)

rm checkeo_persistencia/carpeta1/archivo11
rm -r checkeo_persistencia/carpeta1
umount checkeo_persistencia
rm -r checkeo_persistencia

assert_eq "$expected" "$output" "${RED}Error al montar y desmontar${NC}"

echo " "

echo "Test 25: Write in a file, unmount and mount again"

mkdir solucion_esperada
./fisopfs solucion_esperada # Mount the file system in a new folder

mkdir solucion_esperada/carpeta1
touch solucion_esperada/carpeta1/archivo11

echo "Hola mundo" |  tee solucion_esperada/carpeta1/archivo11 > /dev/null

expected=$( cat solucion_esperada/carpeta1/archivo11)

umount solucion_esperada
rm -r solucion_esperada

mkdir checkeo_persistencia
./fisopfs checkeo_persistencia

output=$( cat checkeo_persistencia/carpeta1/archivo11)


rm checkeo_persistencia/carpeta1/archivo11
rm -r checkeo_persistencia/carpeta1
umount checkeo_persistencia
rm -r checkeo_persistencia

assert_eq "$expected" "$output" "${RED}Error al montar y desmontar${NC}"



# Desmontar el sistema de archivos
if umount prueba_tests; then
  echo ""
else
  echo -e "${RED}Error al desmontar la carpeta: prueba_tests${NC}"
fi

# Eliminar la carpeta creada
if [ -d prueba_tests ]; then
  rm -r prueba_tests
else
  echo "La carpeta no existe: prueba_tests"
fi