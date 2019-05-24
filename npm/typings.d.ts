/// <reference types="node" />
/** A unique identifier for an overlay **/
export type OverlayId = number;

/** URL of the overlay's content **/
export type OverlayUrl = string;

/** Information about an overlay and its position on the screen */
export type OverlayInfo = {
  /** The internal ID of this overlay */
  id: OverlayId;
  /** The source URL that gets loaded into the overlay window */
  url: OverlayUrl;
  /** Width of the window */
  width: number;
  /** Height of the window */
  height: number;
  /** Horizontal position of the window */
  x: number;
  /** Vertical position of the window */
  y: number;
};

/** Native windows handle (WinAPI), encoded as a Node Buffer **/
export type HWND = Buffer;

/**
 * Status of the overlay thread
 */
/*
 * These aren't typos on our end, backend sends this, I can probably look at it later; still, is C.
 * Workaround via enum, do not use these values directly.
 */
export const enum OverlayThreadStatus {
  Starting = 'starting',
  Running = 'runing',
  Stopping = 'stopping',
  Destroyed = 'destoyed',
}

/**
 * Start the overlay thread. This is required before any other operations
 * can be performed (aside from `getStatus`). 
 * Can be done only when getStatus returns `destroyed`. 
 * 
 * Return: 1 if everything went fine. 
 */
export function start(): number;

/**
 * Stop the overlay thread. No operations other than `start` and `getStatus` should be performed
 * after invoking this. To invoce `start` should wait till thread status become `destroyed`
 *
 * Return: 1 if everything went fine.  
 */
export function stop(): number;

/**
 * Returns the number of overlays currently active
 */
export function getCount(): number;

/**
 * Get IDs of all registered overlays
 *
 * @returns An array of overlay IDs
 */
export function getIds(): OverlayId[];

/**
 * Get information about a specific overlay
 *
 * @param id ID of the overlay to get info for
 * @see {OverlayInfo}
 */
export function getInfo(id: OverlayId): OverlayInfo;

/** Show all overlays */
export function show(): void;

/** Hide all overlays */
export function hide(): void;

/**
 * Add an overlay which loads the given URL.
 *
 * @param url The URL to load in the overlay window
 * @returns ID of the overlay that was created
 */
export function add(url: OverlayUrl): OverlayId;

/**
 * Add an overlay with settings
 *
 * @param url URL of the overlay's contents
 * @param x Horizontal position on the screen
 * @param y Vertical position on the screen
 * @param width Window width
 * @param height Window height
 * @returns ID of the overlay that was created
 */
export function addEx(
  url: OverlayUrl,
  x: number,
  y: number,
  width: number,
  height: number,
): OverlayId;

/**
 * Add an overlay to an existing window.
 *
 * @param hwnd The native Windows handle of the window to be set as an overlay
 * @returns ID of the overlay that was created
 * @see {HWND}
 * @example
 * const win = new BrowserWindow({});
 * win.loadURL('https://streamlabs.com');
 * const hwnd = win.getNativeWindowHandle();
 * overlay.addHWND(hwnd);
 */
export function addHWND(hwnd: HWND): OverlayId;

/**
 * Set overlay's position
 *
 * @param id ID of the overlay to be positioned
 * @param x Horizontal position
 * @param y Vertical position
 * @param width Overlay's width
 * @param height Overlay's height
 */
export function setPosition(
  id: OverlayId,
  x: number,
  y: number,
  width: number,
  height: number,
): void;

/**
 * Modify an overlay's URL
 *
 * @param url The new overlay URL
 */
export function setUrl(url: OverlayUrl): void;

/**
 * Reload a specific overlay
 *
 * @param id ID of the overlay to reload
 */
export function reload(id: OverlayId): void;

/**
 * Set an overlay's transparency
 *
 * @param overlayId ID of the overlay to set the transparency for
 * @param transparency A positive integer from 0-255 indicating the transparency to set the overlay to, where 255 indicates opaque
 */
export function setTransparency(overlayId: OverlayId, transparency: number): void;

/**
 * Set an overlay's transparency
 *
 *  @param overlayId ID of the overlay to set
 */
export function paintOverlay(overlayId: OverlayId, width: number, height: number, image: Buffer): void;

/**
 * Remove an overlay
 *
 * @param id ID of the overlay to be removed
 */
export function remove(id: OverlayId): void;

/**
 * Get the status of the overlay thread
 *
 * @see {OverlayThreadStatus}
 */
export function getStatus(): OverlayThreadStatus;

/**
 * Set callback for mouse events 
 *
 * 
 */
export function setMouseCallback(callback: Function): void;

/**
 * Set callback for keyboard events 
 *
 * 
 */
export function setKeyboardCallback(callback: Function): void;


/**
 * Switch on/off interactive mode for overlays
 * In this mode overlay module intercept user keyboard and mouse events and use callbacks to send them to frontend
 * 
 */
export function switchInteractiveMode(callback: Function): void;
