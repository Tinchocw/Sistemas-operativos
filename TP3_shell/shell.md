# shell

### Búsqueda en $PATH
    1. ¿cuáles son las diferencias entre la syscall execve(2) y la familia de wrappers proporcionados por la librería estándar de C (libc) exec(3)?

    Luego del prefijo "exec" podemos notar que se encuentran las letras l, v, e o p.
    Estas letras denotan la funcionalidad particular de la syscall o wrapper, siendo estas:

    l: Los argumentos del archivo a ejecutar se esperan como N punteros a caracteres (strings) siendo el ultimo
    puntero un puntero casteado como (char*) NULL

    v: Los argumentos se pasan como un array de N punteros a caracteres (string) siendo el ultimo un puntero NULL

    e: Se especifica el entorno del nuevo proceso mediante un array de N punteros a caracteres (string) siendo el ultimo un puntero NULL.

    p: Busca el archivo a ejecutar en los directorios especificados por la variable de entorno $PATH.


    2. ¿Puede la llamada a exec(3) fallar? ¿Cómo se comporta la implementación de la shell en ese caso?

    Si, el comando puede fallar en caso de no encontrar el archivo ejecutable, no tener permisos suficientes o problemas con el archivo.
    En caso de un fallo, en la implementación, se indica por pantalla un mensaje de error (stderr).

---

### Procesos en segundo plano

    1. Detallar cuál es el mecanismo utilizado para implementar procesos en segundo plano.
    2. ¿Por qué es necesario el uso de señales?

    // Se responde ambas

    Para implementar procesos en segundo plano, primero debemos cambiar la forma en la que se handlea la senal SIGCHLD mediante
    las syscalls signal o de ser posible es mejor practica utilizar sigaction junto con un stack establecido con
    sigaltstack. Ya que esto nos permite controlar el flujo del programa una vez que un proceso hijo termina pudiendo ser este el proceso en segundo plano u otro en primer plano, por lo cual debemos contemplar ese caso. Para evitar esto lo que hacemos es especificar un Process Group ID (PGID) a los procesos que esten en segundo plano y usar esto para unicamente esperar con waitpid a que terminen los procesos en segundo plano usando los argumentos waitpid(0, &status, WNOHANG), 0 para esperar los procesos que comparten PGID y WNOHANG para que la llamada a waitpid no sea bloqueante en caso de que
    el proceso no haya terminado. En nuestro caso, guardamos el puntero al comando en segundo plano para luego liberar
    la memoria alocada. Por lo cual se uso waitpid(back->pid, &status, WNOHANG).

---

### Flujo estándar
    
    1. Investigar el significado de 2>&1, explicar cómo funciona su forma general

    El significado de 2>&1 sería el siguiente:

    - 2> redirige la salida de error estándar a un archivo o a otro descriptor de archivo.
    - Una primera forma intuitiva de redigir el stderr al stdout podría ser 2>1 pero la terminal interpretaría el 1 como un archivo 
      llamado 1. La forma que tenemos de espicificar de que nos estamos refiriendo a un file descriptor y no a un archivo es con el 
      símbolo &.

    2. Mostrar qué sucede con la salida de cat out.txt en el ejemplo. Luego repetirlo, invirtiendo el orden de las redirecciones (es decir, 2>&1 >out.txt). ¿Cambió algo? Compararlo con el comportamiento en bash(1)

    $ ls -C /home /noexiste >out.txt 2>&1
    $ cat out.txt
    ---????---

    En este ejemplo lo que se hace es redirigir la salida estándar y la salida de error estándar a un archivo llamado out.txt. 
    Quedaría de la siguiente forma:

        $ cat out.txt
        ls: cannot access '/noexiste': No such file or directory
        /home:
        mcwikla

    En el caso de invertir el orden de los redireccionamientos en la shell implementada el resultado sería el mismo. Comparando con 
    bash se puede ver que el resultado no es el mismo.El resultado sería el siguiente:

        $ ls -C /home /noexiste 2>&1 >out.txt
        ls: cannot access '/noexiste': No such file or directory
        $ cat out.txt
        /home:
        mcwikla

    Unicamente en out.txt se guardaría la salida standar y la salida de error standar permaneceria en stderr. La redirección que tiene 
    prioridad es la última que se realiza.


---

### Tuberías múltiples

    1.Investigar qué ocurre con el exit code reportado por la shell si se ejecuta un pipe. 
    ¿Cambia en algo?
    ¿Qué ocurre si, en un pipe, alguno de los comandos falla? Mostrar evidencia (e.g. salidas de terminal) de este comportamiento usando bash. Comparar con su implementación.


    Analizando el exit code al ejecutar uno o más comandos concatenados por pipes, el que queda guardado en $? es el ultimo 
    en ejecutarse (el que más a la derecha se encuentra).

    En el siguiente ejemplo se puede observar que aunque falle el comando ls, que es el primero en ejecutarse y cuyo stdout compone el 
    stdin del comando wc, podemos observar que el wc se ejecuta correctamente y el exit code que se guarda en $? es 0, que corresponde
    al exit code del comando wc.

    ```shell
    $ ls /noexiste | wc -w
    ls: cannot access '/noexiste': No such file or directory
    0
    
    $ echo $?w
    0
    ```
    
    

---

### Variables de entorno temporarias

    1. ¿Por qué es necesario hacerlo luego de la llamada a fork(2)?

    Es necesario hacerlo luego del fork ya que el hijo copia las variables de entorno del padre, por lo que si se realiza antes se modificaran las variables del proceso padre.

    2. En algunos de los wrappers de la familia de funciones de exec(3) (las que finalizan con la letra e), se les puede pasar un tercer argumento (o una lista de argumentos dependiendo del caso), con nuevas variables de entorno para la ejecución de ese proceso. Supongamos, entonces, que en vez de utilizar setenv(3) por cada una de las variables, se guardan en un arreglo y se lo coloca en el tercer argumento de una de las funciones de exec(3).

    ¿El comportamiento resultante es el mismo que en el primer caso? Explicar qué sucede y por qué.
    Describir brevemente (sin implementar) una posible implementación para que el comportamiento sea el mismo.

    Al llamar a setenv, se modifica el entorno del proceso actual, lo cual hace que estas variables esten disponibles
    para los procesos que se ejecuten posteriormente.
    En cambio, el argumento envp lo que hace es crear un entorno temporal para el proceso que se esta por ejecutar lo cual
    no modifica el entorno del proceso actual.

    Para poder hacer que el comportamiento sea el mismo, podriamos en un array meter todas las variables de entorno
    actuales mas las que queremos agregar y pasarlas mediante el argumento env.


---

### Pseudo-variables

    1. Investigar al menos otras tres variables mágicas estándar, y describir su propósito.

    Incluir un ejemplo de su uso en bash (u otra terminal similar).


    $_ --> Contiene el ultimo argumento ingresado.
    uso: si ingresamos en BASH el comando cd PATH/TO/FILE y luego hacemos   echo $_ obtenemos PATH/TO/FILE, si ingresamos seq 10 y luego echo $_  obtenemos 10.

    ```shell
    $ cd PATH/TO/FILE
    $ echo $_
    $ PATH/TO/FILE
    ```


    $$ --> Contiene el PID del BASH actual.
    uso: si ingresamos echo $$ obtendremos un numero de PID, tal como 6424.

    ```shell
    $ echo $$
    $ 6424
    ```


    $! --> Contiene el PID del ultimo proceso corrido en 2do plano.
    uso: si primero ejecutamos el comando sleep 60 & y luego hacemos echo $! obtenemos el PID del sleep, un numero tal como 22461.

    ```shell
    $ sleep 10 &
    $ echo $!
    $ 22461
    ```

---

### Comandos built-in

    1. ¿Entre cd y pwd, alguno de los dos se podría implementar sin necesidad de ser built-in? ¿Por qué? ¿Si la respuesta es sí, cuál es el motivo, entonces, de hacerlo como built-in? (para esta última pregunta pensar en los built-in como true y false)

    Ambos comandos deben ser implementados como built-in.
    'cd' tiene que ser built-in ya que modifica el directorio desde donde se ejecutan los comandos, si no se hace como built-in solo se modificara el directorio del proceso hijo y no afecta al shell.
    'pwd' en este caso no es estrictamente necesario hacerlo como built-in, pero hacerlo nos ahorra el problema de crear un proceso aparte para obtener la informacion del cwd.

---

### Historial

    1. ¿Cuál es la función de los parámetros MIN y TIME del modo no canónico? ¿Qué se logra en el ejemplo dado al establecer a MIN en 1 y a TIME en 0?


---
