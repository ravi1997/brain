# Project Bootstrap (standard conventions)

## Flask-only
- app factory `create_app()`
- gunicorn service/compose uses that
- enable PHI-safe logging rules

## Flask + React
- backend: `/api/*`
- frontend: `/` routes
- CORS configured in dev only

## Always
- health endpoints: `/healthz` and `/readyz`
- request-id propagation at proxy and app
- tests + lint as standard gates
