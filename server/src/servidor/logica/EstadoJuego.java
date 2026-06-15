package servidor.logica;

import java.util.*;
import servidor.logica.modelo.*;

/**
 * LÓGICA — Contiene todo el estado y las reglas del juego.
 *
 * Paradigma Orientado a Objetos:
 *   · Patrón Observer: los ClienteHandler se registran como ObservadorEstado
 *     y reciben el estado serializado cada frame sin que EstadoJuego conozca
 *     los detalles de la comunicación (bajo acoplamiento).
 *   · Patrón Factory: EnemigoFactory crea enemigos según el tipo/fila,
 *     centralizando las reglas de creación.
 *
 * Concurrencia:
 *   El game loop (hilo principal) llama a actualizar() cada ~33 ms.
 *   Los ClienteHandler (hilos separados) llaman a moverIzquierda/disparar.
 *   La serialización ocurre DENTRO del lock pero notificarObservadores() se
 *   llama FUERA: un cliente lento con el búfer TCP lleno no puede bloquear
 *   el game loop.
 */
public class EstadoJuego {

    /* Dimensiones del área de juego (compartidas con el cliente). */
    public static final int ANCHO_PANTALLA    = 1200;
    public static final int ALTO_PANTALLA     = 900;

    /* Configuración de la grilla de enemigos. */
    public static final int FILAS             = 5;
    public static final int COLUMNAS          = 11;
    public static final int ESPACIO_X         = 80;
    public static final int ESPACIO_Y         = 70;
    public static final int INICIO_X_ENEMIGOS = 50;
    public static final int INICIO_Y_ENEMIGOS = 80;
    public static final int BAJADA            = 40; /* píxeles que baja al rebotar el borde */

    /* Bunkers. */
    public static final int NUM_BUNKERS = 4;
    public static final int BUNKER_Y    = 700;

    /* Balas enemigas: máximo simultáneo. */
    public static final int MAX_BALAS_ENEMIGAS = 4;
    private static final int INTERVALO_DISPARO_ENEMIGO = 30; /* frames entre disparos (~1 s a 30 FPS) */

    private List<Jugador>           jugadores;
    private List<Bala>              balas;
    private List<BalaEnemiga>       balasEnemigas;
    private Enemigo[][]             enemigos;
    private List<Enemigo>           enemigosExtra;  /* creados por el admin con CREAR */
    private Ovni                    ovni;
    private List<Bunker>            bunkers;
    private List<ObservadorEstado>  observadores;

    /* Estado del movimiento del bloque enemigo. */
    private int     direccionBloque;    /* 1 = derecha, -1 = izquierda */
    private int     velocidadBloque;    /* píxeles por paso */
    private int     contadorMovimiento; /* frames desde el último movimiento */
    private int     intervaloMovimiento;/* frames entre movimientos (disminuye por oleada) */
    private int     contadorDisparoEnemigo;
    private boolean juegoActivo;

    private final Random random = new Random();

    /* OVNI automático: aparece cada INTERVALO_OVNI frames. */
    private int  contadorOvni;
    private static final int INTERVALO_OVNI = 600; /* ~20 s a 30 FPS */

    public EstadoJuego() {
        jugadores              = new ArrayList<>();
        balas                  = new ArrayList<>();
        balasEnemigas          = new ArrayList<>();
        observadores           = new ArrayList<>();
        enemigosExtra          = new ArrayList<>();
        bunkers                = new ArrayList<>();
        ovni                   = new Ovni();
        direccionBloque        = 1;
        velocidadBloque        = 5;
        contadorMovimiento     = 0;
        intervaloMovimiento    = 5;
        contadorDisparoEnemigo = 0;
        juegoActivo            = true;
        contadorOvni           = 0;
        inicializarEnemigos();
        inicializarBalasEnemigas();
        inicializarBunkers();
    }

    /* ── PATRÓN OBSERVER ────────────────────────────────────────────────── */
    public synchronized void agregarObservador(ObservadorEstado obs) {
        observadores.add(obs);
    }

    public synchronized void eliminarObservador(ObservadorEstado obs) {
        observadores.remove(obs);
    }

    /**
     * Cierra la partida: marca juegoActivo=false, envía un último estado
     * con JUEGO_ACTIVO 0 para que los clientes muestren la pantalla de Game Over,
     * y limpia la lista de observadores.
     */
    public synchronized void cerrar() {
        juegoActivo = false;
        notificarObservadores(serializar());
        observadores.clear();
    }

    /**
     * Notifica a todos los observadores con una copia del array (snapshot)
     * para evitar ConcurrentModificationException y no mantener el lock
     * durante el envío TCP.
     */
    private void notificarObservadores(String estado) {
        ObservadorEstado[] snap;
        synchronized (this) { snap = observadores.toArray(new ObservadorEstado[0]); }
        for (ObservadorEstado obs : snap)
            obs.actualizar(estado);
    }

    /* ── INICIALIZACIÓN ─────────────────────────────────────────────────── */

    /* Usa EnemigoFactory para asignar tipo y puntos según la fila. */
    private void inicializarEnemigos() {
        enemigos = new Enemigo[FILAS][COLUMNAS];
        for (int f = 0; f < FILAS; f++)
            for (int c = 0; c < COLUMNAS; c++)
                enemigos[f][c] = EnemigoFactory.crearPorFila(
                    f,
                    INICIO_X_ENEMIGOS + c * ESPACIO_X,
                    INICIO_Y_ENEMIGOS + f * ESPACIO_Y,
                    c
                );
    }

    private void inicializarBalasEnemigas() {
        balasEnemigas.clear();
        for (int i = 0; i < MAX_BALAS_ENEMIGAS; i++)
            balasEnemigas.add(new BalaEnemiga(i));
    }

    private void inicializarBunkers() {
        bunkers.clear();
        /* Distribuir los bunkers uniformemente en el ancho de la pantalla. */
        int separacion = (ANCHO_PANTALLA - NUM_BUNKERS * Bunker.ANCHO) / (NUM_BUNKERS + 1);
        for (int i = 0; i < NUM_BUNKERS; i++) {
            int x = separacion + i * (Bunker.ANCHO + separacion);
            bunkers.add(new Bunker(i, x, BUNKER_Y));
        }
    }

    /* ── AGREGAR JUGADOR ────────────────────────────────────────────────── */
    public synchronized Jugador agregarJugador() {
        int id = jugadores.size();
        jugadores.add(new Jugador(id));
        balas.add(new Bala(id));
        return jugadores.get(id);
    }

    /* ── ACTUALIZAR (llamado cada frame por el game loop) ───────────────── */
    public void actualizar() {
        String estado;
        synchronized (this) {
            if (!juegoActivo) {
                estado = serializar(); /* seguir enviando JUEGO_ACTIVO 0 hasta desconexión */
            } else {
                moverBloque();
                for (Bala b : balas)             b.actualizar();
                for (BalaEnemiga be : balasEnemigas) be.actualizar(ALTO_PANTALLA);
                actualizarOvniAutomatico();
                ovni.actualizar(ANCHO_PANTALLA);
                dispararEnemigoAutomatico();
                verificarColisiones();
                verificarFinJuego();
                estado = serializar();
            }
        }
        /*
         * Notificar FUERA del lock: si un cliente tiene el búfer TCP lleno,
         * flush() bloquea. Mantenerlo dentro del lock congelaría el game loop.
         */
        notificarObservadores(estado);
    }

    /* ── OVNI AUTOMÁTICO ────────────────────────────────────────────────── */
    /* Aparece cada INTERVALO_OVNI frames con dirección y puntos aleatorios. */
    private void actualizarOvniAutomatico() {
        if (ovni.isActivo()) { contadorOvni = 0; return; }
        contadorOvni++;
        if (contadorOvni >= INTERVALO_OVNI) {
            contadorOvni = 0;
            int dir    = (Math.random() < 0.5) ? 1 : -1;
            int puntos = (int)(Math.random() * 6 + 1) * 50; /* 50-300 pts */
            ovni.aparecer(ANCHO_PANTALLA, dir, puntos);
        }
    }

    /* ── DISPARO ENEMIGO AUTOMÁTICO ─────────────────────────────────────── */
    /**
     * Cada INTERVALO_DISPARO_ENEMIGO frames, el enemigo de frente (más abajo)
     * de una columna aleatoria dispara, si hay un slot de bala enemiga libre.
     */
    private void dispararEnemigoAutomatico() {
        contadorDisparoEnemigo++;
        if (contadorDisparoEnemigo < INTERVALO_DISPARO_ENEMIGO) return;
        contadorDisparoEnemigo = 0;

        BalaEnemiga libre = null;
        for (BalaEnemiga be : balasEnemigas)
            if (!be.isActiva()) { libre = be; break; }
        if (libre == null) return;

        /* Recopilar el primer enemigo vivo de cada columna (el más bajo). */
        List<Enemigo> tiradores = new ArrayList<>();
        for (int c = 0; c < COLUMNAS; c++)
            for (int f = 0; f < FILAS; f++)
                if (enemigos[f][c].isActivo()) { tiradores.add(enemigos[f][c]); break; }

        /* Los enemigos extra del admin también pueden disparar. */
        for (Enemigo e : enemigosExtra)
            if (e.isActivo()) tiradores.add(e);

        if (tiradores.isEmpty()) return;

        Enemigo elegido = tiradores.get(random.nextInt(tiradores.size()));
        int bx = elegido.getX() + Enemigo.ANCHO / 2 - BalaEnemiga.ANCHO / 2;
        int by = elegido.getY() + Enemigo.ALTO;
        libre.disparar(bx, by);
    }

    /* ── MOVIMIENTO DEL BLOQUE ──────────────────────────────────────────── */
    /**
     * El bloque se mueve velocidadBloque píxeles cada intervaloMovimiento frames.
     * Al tocar un borde, invierte la dirección y baja BAJADA píxeles.
     */
    private void moverBloque() {
        contadorMovimiento++;
        if (contadorMovimiento < intervaloMovimiento) return;
        contadorMovimiento = 0;

        boolean borde = false;
        int dx = velocidadBloque * direccionBloque;

        for (int f = 0; f < FILAS; f++)
            for (int c = 0; c < COLUMNAS; c++)
                if (enemigos[f][c].isActivo()) {
                    enemigos[f][c].mover(dx, 0);
                    if (enemigos[f][c].getX() <= 0 ||
                        enemigos[f][c].getX() + Enemigo.ANCHO >= ANCHO_PANTALLA)
                        borde = true;
                }

        for (Enemigo e : enemigosExtra)
            if (e.isActivo()) e.mover(dx, 0);

        if (borde) {
            direccionBloque *= -1;
            for (int f = 0; f < FILAS; f++)
                for (int c = 0; c < COLUMNAS; c++)
                    if (enemigos[f][c].isActivo())
                        enemigos[f][c].mover(0, BAJADA);
            for (Enemigo e : enemigosExtra)
                if (e.isActivo()) e.mover(0, BAJADA);
        }
    }

    /* ── COLISIONES ─────────────────────────────────────────────────────── */
    private void verificarColisiones() {
        /* Balas del jugador contra bunkers, enemigos, extras y OVNI. */
        for (Bala bala : balas) {
            if (!bala.isActiva()) continue;
            Jugador jugador = jugadores.get(bala.getIdJugador());

            boolean hitBunker = false;
            for (Bunker bk : bunkers)
                if (bk.impacto(bala.getX(), bala.getY(), Bala.ANCHO, Bala.ALTO)) {
                    bala.desactivar(); hitBunker = true; break;
                }
            if (hitBunker) continue;

            boolean hitEnemigo = false;
            for (int f = 0; f < FILAS && !hitEnemigo; f++)
                for (int c = 0; c < COLUMNAS && !hitEnemigo; c++) {
                    Enemigo e = enemigos[f][c];
                    if (e.colisionaCon(bala.getX(), bala.getY(), Bala.ANCHO, Bala.ALTO)) {
                        jugador.sumarPuntaje(e.getPuntos());
                        e.destruir();
                        bala.desactivar();
                        verificarVictoria(jugador);
                        hitEnemigo = true;
                    }
                }
            if (hitEnemigo) continue;

            boolean hitExtra = false;
            for (Enemigo e : enemigosExtra)
                if (e.colisionaCon(bala.getX(), bala.getY(), Bala.ANCHO, Bala.ALTO)) {
                    jugador.sumarPuntaje(e.getPuntos());
                    e.destruir(); bala.desactivar();
                    hitExtra = true; break;
                }
            if (hitExtra) continue;

            if (ovni.colisionaCon(bala.getX(), bala.getY(), Bala.ANCHO, Bala.ALTO)) {
                jugador.sumarPuntaje(ovni.getPuntos());
                ovni.destruir(); bala.desactivar();
            }
        }

        /* Balas enemigas contra bunkers y jugadores. */
        for (BalaEnemiga be : balasEnemigas) {
            if (!be.isActiva()) continue;

            boolean hitBunker = false;
            for (Bunker bk : bunkers)
                if (bk.impacto(be.getX(), be.getY(), BalaEnemiga.ANCHO, BalaEnemiga.ALTO)) {
                    be.desactivar(); hitBunker = true; break;
                }
            if (hitBunker) continue;

            for (Jugador j : jugadores)
                if (be.colisionaCon(j.getX(), j.getY(), Jugador.ANCHO, Jugador.ALTO)) {
                    j.perderVida();
                    be.desactivar();
                    if (j.getVidas() <= 0) juegoActivo = false;
                    break;
                }
        }
    }

    /* ── VICTORIA (oleada completada) ───────────────────────────────────── */
    /**
     * Cuando todos los enemigos de la grilla son eliminados, se reinicia
     * la oleada con mayor velocidad (+3 px/paso) y menor intervalo (–1 frame).
     * El jugador gana una vida extra como recompensa.
     */
    private void verificarVictoria(Jugador jugador) {
        for (int f = 0; f < FILAS; f++)
            for (int c = 0; c < COLUMNAS; c++)
                if (enemigos[f][c].isActivo()) return;

        jugador.ganarVida();
        velocidadBloque += 3;
        if (intervaloMovimiento > 2) intervaloMovimiento--;
        inicializarEnemigos();
        for (BalaEnemiga be : balasEnemigas) be.desactivar();
    }

    /* El juego termina si algún enemigo alcanza la fila del jugador. */
    private void verificarFinJuego() {
        for (int f = 0; f < FILAS; f++)
            for (int c = 0; c < COLUMNAS; c++)
                if (enemigos[f][c].isActivo())
                    for (Jugador j : jugadores)
                        if (enemigos[f][c].getY() + Enemigo.ALTO >= j.getY()) {
                            juegoActivo = false;
                            return;
                        }
    }

    /* ── ACCIONES DEL JUGADOR ───────────────────────────────────────────── */
    public synchronized void moverIzquierda(int id) {
        if (id < jugadores.size()) jugadores.get(id).moverIzquierda();
    }

    public synchronized void moverDerecha(int id) {
        if (id < jugadores.size()) jugadores.get(id).moverDerecha(ANCHO_PANTALLA);
    }

    public synchronized void disparar(int id) {
        if (id >= jugadores.size()) return;
        Jugador j = jugadores.get(id);
        /* La bala sale del centro del cañón del jugador. */
        balas.get(id).disparar(j.getX() + Jugador.ANCHO / 2, j.getY());
    }

    /* ── COMANDOS DE ADMINISTRADOR ──────────────────────────────────────── */
    public synchronized void crearEnemigo(int x, int y, String tipo) {
        Enemigo nuevo = EnemigoFactory.crear(tipo, x, y, FILAS, 0);
        enemigosExtra.add(nuevo);
        System.out.println("Enemigo " + tipo + " creado en (" + x + "," + y + ")");
    }

    public synchronized void crearOvni(int direccion, int puntos) {
        ovni.aparecer(ANCHO_PANTALLA, direccion, puntos);
    }

    public synchronized void cambiarVelocidad(int vel) {
        this.velocidadBloque = vel;
    }

    /** Reconstruye o destruye todos los bunkers al porcentaje dado (0–100). */
    public synchronized void cambiarBunkers(int porcentaje) {
        for (Bunker bk : bunkers) bk.setPorcentaje(porcentaje);
        System.out.println("[ADMIN] Bunkers al " + porcentaje + "%");
    }

    /* ── SERIALIZACIÓN ──────────────────────────────────────────────────── */
    /**
     * Produce el estado completo del juego como texto plano, una entidad
     * por línea, dentro de un bloque INICIO_ESTADO / FIN_ESTADO.
     * El cliente parsea este texto para actualizar su vista local.
     */
    public synchronized String serializar() {
        StringBuilder sb = new StringBuilder();
        sb.append("INICIO_ESTADO\n");
        for (Jugador j : jugadores)              sb.append(j.serializar()).append("\n");
        for (Bala b : balas)                     sb.append(b.serializar()).append("\n");
        for (BalaEnemiga be : balasEnemigas)     sb.append(be.serializar()).append("\n");
        for (int f = 0; f < FILAS; f++)
            for (int c = 0; c < COLUMNAS; c++)
                sb.append(enemigos[f][c].serializar()).append("\n");
        /* Tag EXTRA para no confundir con la grilla principal en el parser. */
        for (int i = 0; i < enemigosExtra.size(); i++) {
            Enemigo e = enemigosExtra.get(i);
            sb.append(String.format("EXTRA %d %d %d %d %s",
                i, e.getX(), e.getY(), e.isActivo() ? 1 : 0, e.getTipo().name())).append("\n");
        }
        for (Bunker bk : bunkers) sb.append(bk.serializar()).append("\n");
        sb.append(ovni.serializar()).append("\n");
        sb.append("JUEGO_ACTIVO ").append(juegoActivo ? 1 : 0).append("\n");
        sb.append("FIN_ESTADO\n");
        return sb.toString();
    }

    public boolean isJuegoActivo()      { return juegoActivo; }
    public List<Jugador> getJugadores() { return jugadores;   }
}
