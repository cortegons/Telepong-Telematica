import socket

 

server_ip = "127.0.0.1"  # Cambia esto a la dirección IP del servidor en la nube
server_port = 8080

 

try:
    # Crear un socket de cliente y conectar al servidor
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client_socket.connect((server_ip, server_port))
    print("Conectado al servidor Pong.")

 

    # Lógica de comunicación con el servidor
    while True:
        # Leer entrada del usuario
        message = input("Ingrese un mensaje: ")

 

        # Enviar el mensaje al servidor
        client_socket.send(message.encode())

 

        # Recibir respuesta del servidor
        response = client_socket.recv(1024).decode()
        print("Respuesta del servidor:", response)

 

except ConnectionRefusedError:
    print("No se pudo conectar al servidor. Asegúrate de que el servidor esté en ejecución.")
except Exception as e:
    print("Error de comunicación con el servidor:", str(e))
finally:
    client_socket.close()
