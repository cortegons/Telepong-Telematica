import java.io.*;
import java.net.*;

 

public class PongClient {
    public static void main(String[] args) {
        String serverIP = "13.58.181.109"; // Cambia esto a la dirección IP del servidor en la nube
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
                
                // Recibir información del servidor para el juego
                String gameInfo = in.readLine();
                String[] gameData = gameInfo.split(" ");

                double ballX = Double.parseDouble(gameData[0]);
                double ballY = Double.parseDouble(gameData[1]);
                int paddlePos1 = Integer.parseInt(gameData[2]);
                int paddlePos2 = Integer.parseInt(gameData[3]);

                // Renderizar el juego en la terminal
                renderGame(ballX, ballY, paddlePos1, paddlePos2);
            }

 

        } catch (IOException e) {
            System.err.println("Error de comunicación con el servidor: " + e.getMessage());
        }
    }
    public static void renderGame(double ballX, double ballY, int paddlePos1, int paddlePos2) {
        int fieldWidth = 20; // Ancho del campo de juego para la representación

        // Limpiar la pantalla en cada iteración para la actualización del juego
        System.out.print("\033[H\033[2J");
    
        // Imprimir el campo de juego con la posición de la pelota y las paletas
        for (int i = 0; i < fieldWidth; i++) {
            for (int j = 0; j < fieldWidth; j++) {
                if (i == (int) ballX && j == (int) ballY) {
                   System.out.print("O"); // Pelota
                } else if (i == paddlePos1 || i == paddlePos2) {
                   System.out.print("|"); // Paletas de los jugadores
                } else {
                   System.out.print(" "); // Espacios vacíos
                }
            }
            System.out.println();
        }
    }
}

