package servidor.comunicacion;

import java.io.*;
import java.net.*;
import java.util.*;
import servidor.logica.EstadoJuego;

/**
 * COMUNICACION — Servidor de sockets.
 * Gestiona 1 partida compartida: hasta 2 jugadores + espectadores.
 * Cada cliente jugador controla su propio cañon en el mismo juego.
 */
public class Servidor {

    private static final int PUERTO         = 5000;
    private static final int MAX_JUGADORES  = 2;
    private static final int FPS            = 30;
    private static final long MS_POR_FRAME  = 1000 / FPS;

    private ServerSocket serverSocket;
    private EstadoJuego partida;           // unica partida compartida
    private List<ClienteHandler> clientes;
    private boolean corriendo;

    public Servidor() {
        partida   = new EstadoJuego();
        clientes  = new ArrayList<>();
        corriendo = true;
    }

    public void iniciar() throws IOException {
        serverSocket = new ServerSocket(PUERTO);
        System.out.println("=== spaCEinvaders Servidor ===");
        System.out.println("Puerto: " + PUERTO);
        System.out.println("Max jugadores: " + MAX_JUGADORES + " + espectadores ilimitados");
        System.out.println("Comandos: CREAR x y tipo | OVNI dir pts | VELOCIDAD n");

        new Thread(this::aceptarConexiones).start();
        new Thread(this::manejarAdmin).start();
        gameLoop();
    }

    private void aceptarConexiones() {
        while (corriendo) {
            try {
                Socket socket = serverSocket.accept();

                // ── Leer rol que el cliente envía como primer mensaje ──
                String rolCliente = "JUGADOR";
                try {
                    socket.setSoTimeout(3000);
                    InputStream is = socket.getInputStream();
                    StringBuilder sb = new StringBuilder();
                    int b;
                    while ((b = is.read()) != -1 && b != '\n') {
                        if (b != '\r') sb.append((char) b);
                    }
                    rolCliente = sb.toString().trim();
                    socket.setSoTimeout(0);
                } catch (Exception e) {
                    rolCliente = "JUGADOR";
                }

                boolean esEspectador;

                synchronized (clientes) {
                    if (rolCliente.equalsIgnoreCase("ESPECTADOR")) {
                        esEspectador = true;
                        System.out.println("[SERVIDOR] Espectador conectado -> observando la partida");
                    } else {
                        // Contar jugadores activos en la partida compartida
                        long jugadoresActivos = clientes.stream()
                            .filter(c -> c.isConectado() && !c.isEspectador())
                            .count();
                        if (jugadoresActivos >= MAX_JUGADORES) {
                            esEspectador = true;
                            System.out.println("[SERVIDOR] Partida llena (" + MAX_JUGADORES
                                + " jugadores) -> entra como espectador");
                        } else {
                            esEspectador = false;
                            System.out.println("[SERVIDOR] Nuevo jugador -> partida compartida ("
                                + (jugadoresActivos + 1) + "/" + MAX_JUGADORES + ")");
                        }
                    }

                    ClienteHandler handler = new ClienteHandler(socket, partida, esEspectador);
                    clientes.add(handler);
                    new Thread(handler).start();
                }

            } catch (IOException e) {
                if (corriendo) System.out.println("[ERROR] Aceptando conexion: " + e.getMessage());
            }
        }
    }

    private void gameLoop() {
        while (corriendo) {
            long inicio = System.currentTimeMillis();

            synchronized (clientes) {
                clientes.removeIf(c -> !c.isConectado());
                // Actualizar solo si hay al menos un cliente conectado
                boolean tieneClientes = clientes.stream().anyMatch(ClienteHandler::isConectado);
                if (tieneClientes) {
                    partida.actualizar();
                }
            }

            long espera = MS_POR_FRAME - (System.currentTimeMillis() - inicio);
            if (espera > 0) {
                try { Thread.sleep(espera); }
                catch (InterruptedException e) { Thread.currentThread().interrupt(); }
            }
        }
    }

    private void manejarAdmin() {
        Scanner scanner = new Scanner(System.in);
        System.out.println("Admin listo. Comandos: CREAR x y tipo | OVNI dir pts | VELOCIDAD n | BUNKERS n%");

        while (corriendo) {
            try {
                if (!scanner.hasNextLine()) continue;
                String linea = scanner.nextLine().trim();
                String[] p = linea.split(" ");

                switch (p[0].toUpperCase()) {
                    case "CREAR":
                        // CREAR x y tipo  — Ejemplo: CREAR 300 200 PULPO
                        if (p.length >= 4)
                            partida.crearEnemigo(
                                Integer.parseInt(p[1]), Integer.parseInt(p[2]), p[3]);
                        break;
                    case "OVNI":
                        // OVNI I-D puntos | OVNI D-I puntos
                        if (p.length >= 3) {
                            int dir = p[1].equalsIgnoreCase("I-D") ? 1 : -1;
                            partida.crearOvni(dir, Integer.parseInt(p[2]));
                        }
                        break;
                    case "VELOCIDAD":
                        // VELOCIDAD n
                        if (p.length >= 2)
                            partida.cambiarVelocidad(Integer.parseInt(p[1]));
                        break;
                    case "BUNKERS":
                        // BUNKERS 70% | BUNKERS 40% | BUNKERS 0%
                        if (p.length >= 2) {
                            String pct = p[1].replace("%", "").trim();
                            partida.cambiarBunkers(Integer.parseInt(pct));
                        }
                        break;
                    default:
                        System.out.println("Comando desconocido: " + linea);
                }
            } catch (Exception e) {
                System.out.println("[ERROR] Admin: " + e.getMessage());
            }
        }
    }
}
