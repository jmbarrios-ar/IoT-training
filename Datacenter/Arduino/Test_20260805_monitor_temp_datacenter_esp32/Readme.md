Prompt: Hi, muéstrame un sketch ó código simple de Arduino para el siguiente proyecto: en un centro de datos pequeño donde hay una sala de equipos de refrigeración con tres equipos de enfriamiento perimetral Uniflair Schneider Electric de 50KW funcionando de manera rotativa, se debe monitorear la temperatura y humedad del aire cerca de la zona donde ingresa o succionan estos equipos de refrigeración, debido a que no existen conductos de entrada/salida de aire hacia el exterior. Para esto se pretende utilizar un kit de desarrollo Nodemcu ESP32S conectado a un sensor DHT11, un servidor MQTT y un servidor Home Assistant con su dashboard respectivo.
Cuando se detecta el cruce del umbral de temperatura igual o mayor a 28° Celsius se encienda un led de color rojo, al mismo tiempo se activa una salida para un buzzer o bocina 12VDC y simultáneamente, con una conexión hacia un servidor MQTT ya establecida, en el instante del disparo de detección se debe enviar un Payload=UMBRAL para un tópico=datacenter/refrigeracion/temperatura. 
En otra sala del Centro de Datos sobre el interior del confinamiento donde se encuentran los 16 gabinetes racks (servidores, almacenamiento, dispositivos networking) se necesita colocar el mismo sistema de detección para el mismo valor umbral de temperatura pero con el Payload=RACKS y el tópico=datacenter/servidores/temperatura. 
Esto valores MQTT se utilizarán para enviar notificaciones y visualiza estados en tarjetas del panel mediante automatizaciones en un servidor Home Assistant.
En estado normal de temperatura por abajo del umbral solo se mantiene encendido un led de color verde y el led de color rojo se encuentra apagado y viceversa. De la misma manera con la bocina , solo se activa y suena mientras se está por arriba del umbral.
Sugiere los GPIO más convenientes para el sensor DHT11 y buzzer. También considera en las líneas de código la reconexión automática WiFi sin bloquear el programa, el código comentado mínimamente y su facilidad de mantener.

Respuesta (Claude)
Voy a preparar un sketch para ESP32 que cubra ambos escenarios (sala de refrigeración y sala de racks), usando un bloque de configuración al inicio para elegir cuál topic/payload usar en cada placa.

**GPIOs sugeridos (NodeMCU-32S):**
- DHT11 → **GPIO 4** (pin digital libre, sin función especial de boot)
- LED verde → **GPIO 26**
- LED rojo → **GPIO 27**
- Buzzer/bocina 12VDC (vía módulo relé o transistor NPN, NO directo al GPIO) → **GPIO 25**

Evité GPIO 0, 2, 12, 15 (strapping pins, pueden afectar el boot) y GPIO 34-39 (solo entrada, sin pull-up, no sirven para salidas).**Cómo usarlo en las dos ubicaciones:**

Es el mismo sketch para ambos ESP32; solo cambiás qué línea está descomentada al inicio (`#define UBICACION_REFRIGERACION` o `#define UBICACION_RACKS`) antes de subirlo a cada placa. Eso fija automáticamente el tópico y el payload correctos.

**Puntos clave del diseño:**

- **Reconexión no bloqueante**: ni WiFi ni MQTT usan `delay()`. Ambos se gestionan con `millis()` en cada vuelta del `loop()`, así el sensor y las salidas nunca se congelan mientras se reintenta la conexión.
- **Publicación en el instante del disparo**: el MQTT solo publica en el flanco de subida (cuando pasa de "normal" a "alarma"), tal como pediste — no está mandando el payload repetidamente cada ciclo mientras se mantiene arriba del umbral.
- **LED y buzzer**: reflejan el estado en tiempo real (rojo+buzzer si ≥28°C, verde si está por debajo), independientemente de si hay o no conexión MQTT — la seguridad local no depende de la red.
- **Buzzer 12VDC**: el GPIO del ESP32 da 3.3V y poca corriente, así que esa salida debe ir a un **relé** o **transistor NPN + diodo flyback**, nunca directo al buzzer.

**Librerías a instalar** (Library Manager de Arduino IDE): `PubSubClient` (Nick O'Leary) y `DHT sensor library` + `Adafruit Unified Sensor` (Adafruit).

Como usás Home Assistant con MQTT en `192.168.55.150` (LAN IoT), ya dejé esa IP puesta por defecto en `MQTT_SERVER` — solo confirmá que siga siendo esa tras la migración del broker a la VM de Proxmox que tenías en curso.

¿Querés que te arme también un ejemplo de automatización en Home Assistant (YAML) para las notificaciones y las tarjetas del dashboard a partir de estos dos tópicos?