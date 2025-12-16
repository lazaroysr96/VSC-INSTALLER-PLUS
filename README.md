# VSC-INSTALLER-PLUS

Gestor de instalación de tarballs para Linux con interfaz gráfica Qt5.

## Características

- Instalación de aplicaciones desde archivos tarball locales o URLs
- Creación automática de enlaces simbólicos en `/usr/local/bin`
- Generación de archivos `.desktop` para integración con el menú de aplicaciones
- Registro de aplicaciones instaladas en base de datos SQLite
- Sistema de actualización de aplicaciones existentes
- Interfaz gráfica intuitiva con barra de progreso y logs
- Gestión automática de privilegios de administrador
- Soporte para múltiples editores de código (VS Code, Windsurf, Cursor, etc.)

## Requisitos

- Qt5 (Core, Widgets, Sql, Network)
- CMake 3.16+
- Compilador C++17 compatible
- Sistema Linux x86_64
- Comando `tar` instalado

## Compilación

```bash
mkdir build
cd build
cmake ..
make
```

## Ejecución

```bash
./VSC-INSTALLER-PLUS
```

## Uso

1. Seleccionar fuente del paquete:
   - Archivo local: usar el botón "Examinar..." para seleccionar un tarball
   - URL: ingresar la dirección de descarga directa

2. Configurar opciones de instalación:
   - Ruta de instalación (por defecto: `/opt`)
   - Crear entrada en menú de aplicaciones
   - Crear enlace simbólico en `/usr/local/bin`

3. Hacer clic en "Instalar" para comenzar el proceso

4. Para actualizar aplicaciones existentes, usar el botón "Actualizar"

## Aplicaciones Soportadas

- **Visual Studio Code** (todas las variantes)
- **Windsurf** (AI-powered code editor)
- **Cursor** (AI-powered IDE)
- **VSCodium** (VS Code sin telemetry)
- Cualquier aplicación distribuida en tarball

## Formatos Soportados

- `.tar.gz` / `.tgz`
- `.tar.bz2` / `.tbz2`
- `.tar.xz`
- `.tar`

## Características Avanzadas

### Gestión de Privilegios
- Detección automática de cuándo se necesitan privilegios de administrador
- Escalado de privilegios usando `pkexec`
- Instalación global cuando es necesario

### Detección Inteligente
- Reconocimiento automático de la estructura del tarball
- Identificación de ejecutables para diferentes editores
- Extracción de carpetas específicas (ej: carpeta "Windsurf")

### Integración con el Sistema
- Lanzadores globales en `/usr/share/applications` (root)
- Lanzadores de usuario en `~/.local/share/applications` (usuario normal)
- Enlaces simbólicos en `/usr/local/bin`
- Entradas personalizadas para cada aplicación

## Base de Datos

La aplicación mantiene un registro SQLite en:
`~/.local/share/VSC-INSTALLER-PLUS/apps.db`

## Instalación del Sistema

Para instalar la aplicación en el sistema:

```bash
# Compilar
cd /path/to/VSC-INSTALLER-PLUS
mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local
make

# Instalar (requiere privilegios)
sudo make install
```

## Desinstalación

```bash
sudo make uninstall
```

## Licencia

MIT License - ver archivo LICENSE para detalles.

## Contribuciones

¡Las contribuciones son bienvenidas! Por favor:

1. Fork del repositorio
2. Crear una rama para tu feature (`git checkout -b feature/AmazingFeature`)
3. Commit tus cambios (`git commit -m 'Add some AmazingFeature'`)
4. Push a la rama (`git push origin feature/AmazingFeature`)
5. Abrir un Pull Request

## Issues

Si encuentras algún bug o tienes sugerencias, por favor abre un issue en el repositorio.

## Créditos

- Desarrollado con Qt5/C++
- Icono generado con técnicas de procesamiento de imágenes
- Inspirado en la necesidad de simplificar la instalación de editores de código en Linux
