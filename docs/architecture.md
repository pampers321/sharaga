# 🗺️ Полная схема Telegram-бота WhatsApp-рассылки

---

## 1. Компоненты
```mermaid
flowchart TB
%% ---------- Стили узлов ----------
classDef svc   fill:#e1f5fe,stroke:#0277bd,stroke-width:2px,color:#01579b;
classDef prc   fill:#fffde7,stroke:#f9a825,stroke-width:1px,color:#6d4c41;
classDef store fill:#e8f5e9,stroke:#2e7d32,stroke-width:2px,color:#1b5e20;
classDef ext   fill:#f3e5f5,stroke:#6a1b9a,stroke-width:2px,color:#4a148c;
classDef ci    fill:#e3f2fd,stroke:#1565c0,stroke-width:2px,color:#003c8f;
classDef mid   fill:#ede7f6,stroke:#512da8,stroke-width:1px,color:#311b92;
classDef fsm   fill:#fff3e0,stroke:#ef6c00,stroke-width:1px,color:#bf360c;

%% ---------- (0) Легенда ----------
subgraph "Legend (click to expand)"
    direction LR
    L1[Service]:::svc --> L2[Process]:::prc --> L3[(Storage)]:::store --> L4[External]:::ext --> L5[CI/CD]:::ci --> L6[Middleware]:::mid --> L7[State]:::fsm
end

%% ---------- (1) CI/CD ----------
subgraph "CI-Pipeline"
    direction LR
    ci_lint["Lint & Ruff"]:::ci --> ci_test["PyTest"]:::ci --> ci_build["Docker Build"]:::ci --> ci_push["Push image"]:::ci --> ci_deploy["SSH Deploy"]:::ci
end
docker_reg[(Docker Registry)]:::ext
ci_push -- image --> docker_reg
ci_deploy -- pull --> docker_reg
ci_deploy -- exec --> deploy_sh[[deploy.sh]]:::prc

%% ---------- (2) Docker-Stack ----------
subgraph "Docker-Stack work_auto_$REV"
    direction TB

    subgraph "Network : appnet"
        redis_srv[(redis:7)]:::svc
        volume_redis[(redis_data)]:::store
        redis_srv -- persists --> volume_redis
    end

    subgraph "Service : bot"
        bot_ctr[bot container]:::svc
    end

    subgraph "Service : task_runner"
        runner_ctr[task_runner container]:::svc
    end

    backup[backup cron]:::svc

    volume_data[(bind ./data)]:::store
    bot_ctr -- bind --> volume_data
    runner_ctr -- bind --> volume_data
    backup   -- bind --> volume_data

    bot_ctr -->|depends_on| redis_srv
    runner_ctr -->|depends_on| redis_srv
end

%% ---------- (3) Bot runtime ----------
bot_ctr --> run_bot["run_bot.py"]:::prc
run_bot --> dispatcher["Aiogram Dispatcher"]:::prc
dispatcher --> mid_maint[MaintenanceMW]:::mid
dispatcher --> mid_album[AlbumMW]:::mid
run_bot --> robokassa_srv["Robokassa Webhook"]:::prc
run_bot --> task_mgr[AsyncTaskManager]:::prc
run_bot --> redis_fsm[RedisStorage]:::prc

subgraph "Handlers"
    direction LR
    h_nav[navigation]:::prc
    h_admin[admin]:::prc
    h_user[user_data]:::prc
    h_text[text]:::prc
    h_images[images]:::prc
    h_groups[groups]:::prc
    h_mail[mailing]:::prc
    h_sub[subscription]:::prc
    h_misc[misc]:::prc
end
dispatcher --> h_nav & h_admin & h_user & h_text & h_images & h_groups & h_mail & h_sub & h_misc

%% ---------- (3.1) FSM ----------
subgraph "FSM (StatesGroup)"
    direction LR
    st_token[new_token]:::fsm
    st_profile[new_profile_id]:::fsm
    st_text[text]:::fsm
    st_groups_new[groups_input]:::fsm
    st_groups_add[add_groups_input]:::fsm
    st_images[images_input]:::fsm
    st_confirm[confirm_mailing]:::fsm
end
h_user --> st_token & st_profile
h_text --> st_text
h_groups --> st_groups_new & st_groups_add
h_images --> st_images
h_mail --> st_confirm

%% ---------- (4) task_runner runtime ----------
runner_ctr --> runner_main[task_runner.py]:::prc
runner_main --> blpop[BLPOP mailing_tasks_queue]:::prc
runner_main --> health_tick[Health Ping]:::prc
blpop --> mailing_task[async_tasks.mailing_task]:::prc
mailing_task --> run_core[core.run_core]:::prc

subgraph "run_core()"
    direction LR
    db_layer[DataBaseInteraction]:::prc
    wap_client[WappiApiClient]:::prc
    hb_lock[AsyncHeartbeatLock]:::prc
    helper_antidup[antidup.py]:::prc
end
run_core --> db_layer & wap_client & hb_lock & helper_antidup
wap_client --> WappiAPI[(Wappi API)]:::ext

%% ---------- (5) Хранилища ----------
redis_srv -.-> queue[mailing_tasks_queue]:::store
redis_srv -.-> lock_keys[mailing_lock_*]:::store
redis_srv -.-> cancel_keys[cancel_mailing_*]:::store
redis_srv -.-> health_key[task_runner_health]:::store
db_layer <--> sqlite_db[(SQLite users.db)]:::store
backup -- dump --> dumps[(users.db.dumpYYYYMMDD)]:::store

%% ---------- (6) Оплата ----------
h_sub -- generate_link --> robokassa_client[RobokassaClient]:::prc
robokassa_client --> Robokassa[(Robokassa)]:::ext
Robokassa -- Result_URL --> robokassa_srv
robokassa_srv --> db_layer

%% ---------- (7) Внешние сервисы ----------
bot_ctr -- poll/send --> TG_API[(Telegram API)]:::ext
TG_API -- msgs --> end_users((End Users)):::ext
WappiAPI -- deliver --> WhatsApp((WhatsApp Groups)):::ext

%% ---------- (8) Ops-скрипты ----------
monitor_sh[[monitor.sh]]:::prc --> redis_srv
diagnose_sh[[diagnose-runner.sh]]:::prc --> runner_ctr
clean_old_sh[[clean_old_stacks.sh]]:::prc --> docker_reg
deploy_sh --> clean_old_sh & monitor_sh & diagnose_sh
helper_logging[logging_utils]:::prc --> log_file[(debug.log)]:::store

%% ---------- Node-классы ----------
class bot_ctr,runner_ctr,backup svc
class redis_srv,queue,lock_keys,cancel_keys,health_key,sqlite_db,volume_redis,volume_data,dumps store
class docker_reg,TG_API,WappiAPI,Robokassa,WhatsApp ext
