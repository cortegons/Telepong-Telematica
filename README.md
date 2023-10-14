# Telepong---Telematica

Gustavo Adolfo López García 

Camilo Ortegón Saugster

# Introducción
El proyecto consiste en la implementación del popular juego Pong haciendo uso de protocolos de red que nos permitan la conexión de dos o más
jugadores al tiempo en diferentes salas, haciendo uso de sockets que permitan la conexión entre clientes y servidor.

# Desarrollo

Para el desarrollo del cliente se uso un código escrito en C y ejecutado sobre la instancia en AWS. Este código tiene toda la configuración del servidor, la actualización del juego en tiempo real y la verificación de las coliciones de la pelota como la dirección en la que esta saldra.

Mientras que el cliente se desarrollo en Java y este lo que hace es establecer la comunicación con el servidor mediante la creación del debido socket. Enviar las jugadas de cada uno de los usuarios al servidor y la validación de estos mismos.

# Conclusiones
Los sockets son una tecnología muy utilizada y muy útil a la hora de realizar conexiones donde se esperan peticiones y respuestas a tiempo real, como por ejemplo videojuegos o chats en vivo.
Nos permiten un muyu útil intercambio de datos entre diferentes canales y redes.
# Referencias
https://www.youtube.com/watch?v=GFTsU_XRXEg

https://docs.aws.amazon.com/efs/latest/ug/gs-step-one-create-ec2-resources.html
