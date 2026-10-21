"""Kellersteuerung: standalone Home Assistant sidebar panel.

Uses already existing entities. Does not query the tank sensor directly.
"""
from __future__ import annotations

import logging
from pathlib import Path

import voluptuous as vol
from homeassistant.components import frontend, panel_custom
from homeassistant.components.http import StaticPathConfig
from homeassistant.core import HomeAssistant
from homeassistant.helpers import config_validation as cv

DOMAIN = "heizoel_dashboard_panel"
VERSION = "2.4.0"
_LOGGER = logging.getLogger(__name__)
PANEL_URL = "/heizoel-dashboard-assets"
DEFAULT_PATH = "heizoel-monitor"

PANEL_SCHEMA = vol.Schema(
    {
        vol.Optional("sidebar_title", default="Kellersteuerung"): cv.string,
        vol.Optional("sidebar_icon", default="mdi:home-thermometer"): cv.icon,
        vol.Optional("url_path", default=DEFAULT_PATH): cv.string,
        vol.Optional("entities", default={}): {cv.string: cv.entity_id},
    },
    extra=vol.PREVENT_EXTRA,
)

CONFIG_SCHEMA = vol.Schema(
    {vol.Optional(DOMAIN): PANEL_SCHEMA}, extra=vol.ALLOW_EXTRA
)


async def async_setup(hass: HomeAssistant, config: dict) -> bool:
    """Register a non-iframe panel, exposing the native HA hass object."""
    options = config.get(DOMAIN)
    if options is None:
        return True

    url_path = options["url_path"].strip("/")
    if not url_path or "/" in url_path:
        _LOGGER.error("Invalid Kellersteuerung url_path: %r", url_path)
        return False

    if url_path in hass.data.get("frontend_panels", {}):
        _LOGGER.error("Panel URL path '%s' is already used", url_path)
        return False

    asset = Path(__file__).parent / "frontend" / "heizoel-panel.js"
    await hass.http.async_register_static_paths(
        [StaticPathConfig(f"{PANEL_URL}/heizoel-panel.js", str(asset), False)]
    )
    await panel_custom.async_register_panel(
        hass=hass,
        frontend_url_path=url_path,
        webcomponent_name="heizoel-monitor-panel",
        module_url=f"{PANEL_URL}/heizoel-panel.js?v={VERSION.replace('.', '_')}",
        sidebar_title=options["sidebar_title"],
        sidebar_icon=options["sidebar_icon"],
        config={"entities": dict(options["entities"])},
        embed_iframe=False,
        require_admin=False,
    )
    _LOGGER.info("Kellersteuerung panel registered at /%s", url_path)
    return True
