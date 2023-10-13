import java.io.*;
import java.net.*;

 

public class PongClient {
    public static void main(String[] args) {
        String serverIP = "127.0.0.1"; // Cambia esto a la dirección IP del servidor en la nube
        int serverPort = 8080;

 

        try {
            // Crear un socket de cliente y conectar al servidor
            Socket socket = new Socket(serverIP, serverPort);
            System.out.println("Conectado al servidor Pong.");

 

            // Configurar flujos de entrada y salida
            BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            PrintWriter out = new PrintWriter(socket.getOutputStream(), true);

 

            // Lógica de comunicación con el servidor
            BufferedReader userInput = new BufferedReader(new InputStreamReader(System.in));
            String message;

 

            while (true) {
                // Leer entrada del usuario
                System.out.print("Ingrese un mensaje: ");
                message = userInput.readLine();

 

                // Enviar el mensaje al servidor
                out.println(message);

 

                // Recibir respuesta del servidor
                String response = in.readLine();
                System.out.println("Respuesta del servidor: " + response);
            }

 

        } catch (IOException e) {
            System.err.println("Error de comunicación con el servidor: " + e.getMessage());
        }
    }
}
