import java.io.*;
import java.net.*;

public class PongClient {
    public static void main(String[] args) {
        String serverIP = "13.58.181.109"; // Cambia esto a la dirección IP del servidor en la nube
        int serverPort = 8888;

        try {
            Socket socket = new Socket(serverIP, serverPort);
            System.out.println("Conectado al servidor Pong.");

            BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            PrintWriter out = new PrintWriter(socket.getOutputStream(), true);

            BufferedReader userInput = new BufferedReader(new InputStreamReader(System.in));

            // Obtener los datos de usuario del cliente
            System.out.print("Ingrese su nombre de usuario: ");
            String username = userInput.readLine();
            out.println(username); // Enviar nombre de usuario al servidor

            System.out.print("Ingrese su contraseña: ");
            String password = userInput.readLine();
            out.println(password); // Enviar contraseña al servidor

            while (true) {
                try {
                    String gameInfo = in.readLine();
                    if (gameInfo != null) {
                        renderGame(gameInfo);
                    }
                } catch (IOException e) {
                    System.err.println("Error al recibir datos del servidor: " + e.getMessage());
                }

                // Leer entrada del usuario y enviar al servidor
                System.out.print("Ingrese un comando (W para arriba, S para abajo): ");
                String command = userInput.readLine();

                if (command.equalsIgnoreCase("W") || command.equalsIgnoreCase("S")) {
                    out.println(command); // Enviar el comando al servidor
                } else {
                    System.out.println("Comando inválido. Use 'W' para arriba o 'S' para abajo.");
                    continue; // Volver a pedir un nuevo comando
                }
            }

        } catch (IOException e) {
            System.err.println("Error de comunicación con el servidor: " + e.getMessage());
        }
    }

    // Método para renderizar el juego en la terminal
    public static void renderGame(String gameInfo) {
        String[] gameData = gameInfo.split(" ");

        // Obtener datos del juego
        double ballX = Double.parseDouble(gameData[0]);
        double ballY = Double.parseDouble(gameData[1]);
        int paddlePos1 = Integer.parseInt(gameData[2]);
        int paddlePos2 = Integer.parseInt(gameData[3]);

        int fieldWidth = 10; // Ancho del campo de juego para la representación

        // Limpiar la pantalla en cada iteración para la actualización del juego
        System.out.print("\033[H\033[2J");

        // Representar el campo de juego con la posición de la pelota y las paletas
        for (int i = 0; i < fieldWidth; i++) {
            for (int j = 0; j < fieldWidth; j++) {
                if (j == 0 || j == fieldWidth - 1) {
                    System.out.print("|"); // Paredes del campo
                } else if ((int) ballX == j && (int) ballY == i) {
                    System.out.print("O"); // Pelota
                } else if (i == paddlePos1 && (j == 1 || j == 2)) {
                    System.out.print("|"); // Paleta del jugador 1
                } else if (i == paddlePos2 && (j == fieldWidth - 3 || j == fieldWidth - 2)) {
                    System.out.print("|"); // Paleta del jugador 2
                } else {
                    System.out.print(" "); // Espacios vacíos dentro del campo
                }
            }
            System.out.println();
        }
    }
}