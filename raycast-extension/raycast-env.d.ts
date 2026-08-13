/// <reference types="@raycast/api">

/* 🚧 🚧 🚧
 * This file is auto-generated from the extension's manifest.
 * Do not modify manually. Instead, update the `package.json` file.
 * 🚧 🚧 🚧 */

/* eslint-disable @typescript-eslint/ban-types */

type ExtensionPreferences = {}

/** Preferences accessible in all the extension's commands */
declare type Preferences = ExtensionPreferences

declare namespace Preferences {
  /** Preferences accessible in the `toggle-night-watch` command */
  export type ToggleNightWatch = ExtensionPreferences & {}
  /** Preferences accessible in the `night-watch-menu` command */
  export type NightWatchMenu = ExtensionPreferences & {}
}

declare namespace Arguments {
  /** Arguments passed to the `toggle-night-watch` command */
  export type ToggleNightWatch = {}
  /** Arguments passed to the `night-watch-menu` command */
  export type NightWatchMenu = {}
}
