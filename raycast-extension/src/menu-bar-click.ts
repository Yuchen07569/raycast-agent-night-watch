import { StatusKind } from "./status";

export function createSingleFlight<T>() {
  let current: Promise<T> | undefined;
  return (operation: () => Promise<T>): Promise<T> => {
    current ??= operation();
    return current;
  };
}

export function shouldToggleFromMenuBar(
  launchType: string,
  alreadyActivated: boolean,
  statusKind: StatusKind,
): boolean {
  return (
    launchType === "userInitiated" &&
    alreadyActivated &&
    (statusKind === "off" || statusKind === "on-owned")
  );
}
