/** A unique identifier for an overlay **/
export type OverlayId = string;

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

/**
 * Status of the overlay thread
 */
/*
 * These aren't typos on our end, backend sends this, I can probably look at it later; still, is C.
 * Workaround via enum, do not use these values directly.
 */
export enum OverlayThreadStatus {
  Starting = 'starting',
  Running = 'runing',
  Stopping = 'stopping',
  Destroyed = 'destoyed',
}

/**
 * Start the overlay thread. This seems to be required before any other operations
 * can be performed (aside from `getStatus`).
 */
export function start(): void;

/**
 * Stop the overlay thread. No operations other than `start` and `getStatus` should be performed
 * after invoking this.
 */
export function stop(): void;

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
 * @param transparency A positive integer from 0-255 indicating the transparency to set the overlay to, where 255 indicates opaque
 */
export function setTransparency(transparency: number): void;

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
