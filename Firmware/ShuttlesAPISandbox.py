#!/usr/bin/env python3
"""Test the MBTA v3 predictions API for route 66 at N Harvard St @ Oxford St.

Default query matches the ESP32 firmware config:
  route 66, stop 2559, sort by arrival_time, limit 10

Usage:
  python3 test_mbta_api.py
  python3 test_mbta_api.py --api-key YOUR_MBTA_KEY
  python3 test_mbta_api.py --route 66 --stop 2559
"""

from __future__ import annotations

import argparse
import datetime as dt
import json
import sys
from typing import Any
from urllib.parse import urlencode
from urllib.request import Request, urlopen
from urllib.error import HTTPError, URLError

DEFAULT_ROUTE_ID = "66"
DEFAULT_STOP_ID = "2559"  # N Harvard St @ Oxford St
MBTA_BASE_URL = "https://api-v3.mbta.com/predictions"


def parse_mbta_time(value: str | None) -> dt.datetime | None:
    """Parse an MBTA ISO-8601 timestamp such as 2026-07-04T22:14:00-04:00."""
    if not value:
        return None
    try:
        return dt.datetime.fromisoformat(value.replace("Z", "+00:00"))
    except ValueError:
        return None


def minutes_until(t: dt.datetime, now: dt.datetime | None = None) -> int:
    if now is None:
        now = dt.datetime.now(t.tzinfo)
    return round((t - now).total_seconds() / 60)


def build_url(route_id: str, stop_id: str, limit: int) -> str:
    params = {
        "sort": "arrival_time",
        "filter[route]": route_id,
        "filter[stop]": stop_id,
        "page[limit]": str(limit),
    }
    return f"{MBTA_BASE_URL}?{urlencode(params)}"


def fetch_json(url: str, api_key: str | None = None, timeout_s: int = 10) -> dict[str, Any]:
    headers = {
        "Accept": "application/vnd.api+json",
        "User-Agent": "mbta-esp32-api-test/1.0",
    }
    if api_key:
        headers["x-api-key"] = api_key

    req = Request(url, headers=headers, method="GET")
    try:
        with urlopen(req, timeout=timeout_s) as resp:
            body = resp.read().decode("utf-8")
            return json.loads(body)
    except HTTPError as e:
        body = e.read().decode("utf-8", errors="replace")
        raise RuntimeError(f"HTTP {e.code}: {body}") from e
    except URLError as e:
        raise RuntimeError(f"Network error: {e.reason}") from e
    except json.JSONDecodeError as e:
        raise RuntimeError(f"Response was not valid JSON: {e}") from e


def test_predictions(data: dict[str, Any], route_id: str, stop_id: str) -> int:
    predictions = data.get("data", [])
    if not isinstance(predictions, list):
        print("FAIL: response field 'data' is not a list")
        return 1

    print(f"OK: API returned {len(predictions)} prediction(s)")

    bad_rows = 0
    usable_rows = 0

    for i, prediction in enumerate(predictions, start=1):
        relationships = prediction.get("relationships", {})
        attrs = prediction.get("attributes", {})

        got_route = relationships.get("route", {}).get("data", {}).get("id")
        got_stop = relationships.get("stop", {}).get("data", {}).get("id")
        arrival_raw = attrs.get("arrival_time")
        departure_raw = attrs.get("departure_time")
        status = attrs.get("status")

        if got_route != route_id or got_stop != stop_id:
            bad_rows += 1
            print(f"FAIL row {i}: route={got_route!r}, stop={got_stop!r}")
            continue

        event_time = parse_mbta_time(arrival_raw) or parse_mbta_time(departure_raw)
        if event_time is None:
            print(f"row {i}: route {got_route}, stop {got_stop}, no arrival/departure time, status={status!r}")
            continue

        usable_rows += 1
        mins = minutes_until(event_time)
        label = "ARRIVE" if mins <= 0 else f"{mins} min"
        print(
            f"row {i}: route {got_route}, stop {got_stop}, "
            f"time={event_time.isoformat()}, display={label}, status={status!r}"
        )

    if bad_rows:
        print(f"FAIL: {bad_rows} prediction(s) did not match route={route_id}, stop={stop_id}")
        return 1

    if not predictions:
        print("PASS: endpoint works, but MBTA currently reports no predictions for this stop/route")
        return 0

    if usable_rows == 0:
        print("WARN: endpoint works, but no returned prediction had an arrival_time or departure_time")
        return 0

    print(f"PASS: all returned predictions match route={route_id}, stop={stop_id}")
    return 0


def main() -> int:
    parser = argparse.ArgumentParser(description="Test MBTA route/stop predictions API.")
    parser.add_argument("--route", default=DEFAULT_ROUTE_ID, help="MBTA route ID, default: 66")
    parser.add_argument("--stop", default=DEFAULT_STOP_ID, help="MBTA stop ID, default: 2559")
    parser.add_argument("--limit", type=int, default=10, help="Max predictions to request")
    parser.add_argument("--api-key", default=None, help="Optional MBTA API key")
    parser.add_argument("--dump-json", action="store_true", help="Print raw JSON response")
    args = parser.parse_args()

    url = build_url(args.route, args.stop, args.limit)
    print(f"GET {url}")

    try:
        data = fetch_json(url, api_key=args.api_key)
    except RuntimeError as e:
        print(f"FAIL: {e}")
        return 2

    if args.dump_json:
        print(json.dumps(data, indent=2, sort_keys=True))

    return test_predictions(data, args.route, args.stop)


if __name__ == "__main__":
    sys.exit(main())
