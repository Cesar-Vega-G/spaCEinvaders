package servidor.comunicacion;

import java.io.*;
import java.net.*;
import java.util.*;
import servidor.logica.EstadoJuego;

/**
 * COMUNICACION — Servidor de sockets TCP para spaCEinvaders.
 *
 * Responsabilidades:
 *   · Aceptar conexiones de jugadores y espectadores en el puerto 5000.
 *   · Gestionar N partidas independientes (una por jugador conectado).
 *   · Ejecutar el game loop a 30 FPS y delegar a EstadoJuego.actualizar().
 *   · Controlar slots: máximo 1 jugador de teclado y 1 de Pico simultáneamente.
 *   · Permitir comandos de administrador por consola (CREAR, VELOCIDAD, etc.).
 *
 * Hilos:
 *   · main          → gameLoop() (tick de 30 FPS)
 *   · aceptarConexiones → escucha nuevas conexiones (bloqueante)
 *   · manejarAdmin  → lee comandos de consola
 *   · 1 hilo por ClienteHandler (jugador o espectador)
 */
public class Servidor {

    private static final int PUERTO        = 5000;
    private static final int FPS           = 30;
    private static final long MS_POR_FRAME = 1000 / FPS;

    private ServerSocket serverSocket;
    private final Map<Integer, EstadoJuego> partidas = new LinkedHashMap<>();
    private final List<ClienteHandler>      clientes = new ArrayList<>();
    private int     nextId    = 0;
    private boolean corriendo = true;

    /* Slots de control: solo un jugador por tipo a la vez. */
    private boolean tecladoConectado = false;
    private boolean picoConectado    = false;

    public void iniciar() throws IOException {
        serverSocket = new ServerSocket(PUERTO);
        System.out.println("=== spaCEinvaders Servidor ===");
        System.out.println("Puerto: " + PUERTO);
        System.out.println("1 jugador por partida | espectadores ilimitados");
        System.out.println("Comandos: CREAR x y tipo | OVNI dir pts | VELOCIDAD n | BUNKERS n%");

        new Thread(this::aceptarConexiones).start();
        new Thread(this::manejarAdmin).start();
        gameLoop(); /* bloquea el hilo principal */
    }

    /* ── ACEPTAR CONEXIONES ─────────────────────────────────────────────── */
    private void aceptarConexiones() {
        while (corriendo) {
            try {
                Socket socket = serverSocket.accept();
                String rol = leerRol(socket);

                /* Los espectadores no ocupan slot; se registran en cualquier partida. */
                if (rol.equalsIgnoreCase("ESPECTADOR")) {
                    System.out.println("[SERVIDOR] Espectador conectado");
                    ClienteHandler h = new ClienteHandler(socket, null, true, this, null);
                    synchronized (clientes) { clientes.add(h); }
                    new Thread(h).start();
                    continue;
                }

                boolean esPico = rol.equalsIgnoreCase("JUGADOR_PICO");
                String tipo    = esPico ? "PICO" : "TECLADO";

                ClienteHandler handler;
                synchronized (partidas) {
                    boolean ocupado = esPico ? picoConectado : tecladoConectado;
                    if (ocupado) {
                        /* Rechazar con SLOT_OCUPADO y cerrar el socket inmediatamente. */
                        try {
                            PrintWriter pw = new PrintWriter(socket.getOutputStream(), true);
                            pw.println("SLOT_OCUPADO");
                            socket.close();
                        } catch (Exception ex) { /* ignorar */ }
                        System.out.println("[SERVIDOR] Slot " + tipo + " ocupado - rechazo");
                        continue;
                    }
                    /* Reservar el slot y crear una nueva partida independiente. */
                    if (esPico) picoConectado = true;
                    else tecladoConectado = true;

                    EstadoJuego nueva = new EstadoJuego();
                    int id = nextId++;
                    partidas.put(id, nueva);
                    handler = new ClienteHandler(socket, nueva, false, this, tipo);
                    handler.setPartidaId(id);
                    System.out.println("[SERVIDOR] Jugador " + tipo + " -> partida " + id + " creada");
                }

                synchronized (clientes) { clientes.add(handler); }
                new Thread(handler).start();

            } catch (IOException e) {
                if (corriendo) System.out.println("[ERROR] Aceptando conexion: " + e.getMessage());
            }
        }
    }

    /* Libera el slot cuando el jugador se desconecta (llamado por ClienteHandler). */
    public void liberarSlot(String tipo) {
        if (tipo == null) return;
        synchronized (partidas) {
            if ("TECLADO".equals(tipo)) tecladoConectado = false;
            else if ("PICO".equals(tipo))  picoConectado    = false;
        }
        System.out.println("[SERVIDOR] Slot " + tipo + " liberado");
    }

    /* Lee el rol enviado por el cliente al conectarse (timeout 3 s). */
    private String leerRol(Socket socket) {
        try {
            socket.setSoTimeout(3000);
            InputStream is = socket.getInputStream();
            StringBuilder sb = new StringBuilder();
            int b;
            while ((b = is.read()) != -1 && b != '\n')
                if (b != '\r') sb.append((char) b);
            socket.setSoTimeout(0);
            return sb.toString().trim();
        } catch (Exception e) {
            return "JUGADOR";
        }
    }

    /* ── ACCESO SEGURO A PARTIDAS (para ClienteHandler) ─────────────────── */
    public int enviarListaPartidas(PrintWriter salida) {
        synchronized (partidas) {
            salida.println("PARTIDAS " + partidas.size());
            for (Integer id : partidas.keySet())
                salida.println("PARTIDA " + id);
            return partidas.size();
        }
    }

    public EstadoJuego getPartida(int id) {
        synchronized (partidas) { return partidas.get(id); }
    }

    public void eliminarPartida(int id) {
        synchronized (partidas) { partidas.remove(id); }
        System.out.println("[SERVIDOR] Partida " + id + " eliminada");
    }

    /* ── GAME LOOP (30 FPS) ─────────────────────────────────────────────── */
    private void gameLoop() {
        while (corriendo) {
            long inicio = System.currentTimeMillis();

            /* Snapshot para evitar ConcurrentModificationException durante actualizar(). */
            List<EstadoJuego> snapshot;
            synchronized (partidas) { snapshot = new ArrayList<>(partidas.values()); }
            for (EstadoJuego p : snapshot) p.actualizar();

            /* Limpiar handlers desconectados. */
            synchronized (clientes) { clientes.removeIf(c -> !c.isConectado()); }

            long espera = MS_POR_FRAME - (System.currentTimeMillis() - inicio);
            if (espera > 0)
                try { Thread.sleep(espera); }
                catch (InterruptedException e) { Thread.currentThread().interrupt(); }
        }
    }

    /* ── COMANDOS DE ADMINISTRADOR ──────────────────────────────────────── */
    private void manejarAdmin() {
        Scanner scanner = new Scanner(System.in);
        System.out.println("Admin listo. Comandos: CREAR x y tipo | OVNI dir pts | VELOCIDAD n | BUNKERS n%");

        while (corriendo) {
            try {
                if (!scanner.hasNextLine()) continue;
                String linea = scanner.nextLine().trim();
                String[] p   = linea.split(" ");

                List<EstadoJuego> snap;
                synchronized (partidas) { snap = new ArrayList<>(partidas.values()); }

                switch (p[0].toUpperCase()) {
                    case "CREAR":     /* Crear enemigo extra en (x,y) de tipo CALAMAR/CANGREJO/PULPO */
                        if (p.length >= 4)
                            for (EstadoJuego eg : snap)
                                eg.crearEnemigo(Integer.parseInt(p[1]), Integer.parseInt(p[2]), p[3]);
                        break;
                    case "OVNI":      /* Lanzar OVNI con dirección (I-D / D-I) y puntos */
                        if (p.length >= 3) {
                            int dir = p[1].equalsIgnoreCase("I-D") ? 1 : -1;
                            for (EstadoJuego eg : snap)
                                eg.crearOvni(dir, Integer.parseInt(p[2]));
                        }
                        break;
                    case "VELOCIDAD": /* Cambiar velocidad del bloque enemigo en píxeles */
                        if (p.length >= 2)
                            for (EstadoJuego eg : snap)
                                eg.cambiarVelocidad(Integer.parseInt(p[1]));
                        break;
                    case "BUNKERS":   /* Reconstruir/destruir bunkers: 100% = completos, 0% = destruidos */
                        if (p.length >= 2) {
                            String pct = p[1].replace("%", "").trim();
                            for (EstadoJuego eg : snap)
                                eg.cambiarBunkers(Integer.parseInt(pct));
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
