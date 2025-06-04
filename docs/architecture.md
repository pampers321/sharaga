# Архитектура WhatsApp-рассылки

## 1. Компоненты
```mermaid
graph TB
    subgraph "External Services"
        TG[Telegram API]
        WAPPI[Wappi API<br/>WhatsApp Gateway]
        ROBO[Robokassa<br/>Payment Gateway]
    end

    subgraph "Docker Stack work_auto_REV"
        subgraph "Bot Service"
            BOT[Telegram Bot<br/>aiogram]
            HANDLERS[Handlers<br/>- Navigation<br/>- Mailing<br/>- Groups<br/>- Images<br/>- Subscription]
            FSM[FSM States]
        end

        subgraph "Task Runner Service"
            RUNNER[Task Runner<br/>asyncio]
            PROCESSOR[Task Processor]
            HEALTH[Health Check]
        end

        subgraph "Data Layer"
            REDIS[(Redis<br/>- Task Queue<br/>- Locks<br/>- Cache<br/>- Cancellation Flags)]
            SQLITE[(SQLite<br/>users.db)]
        end
    end

    subgraph "CI/CD"
        GHA[GitHub Actions]
        DOCKER[Docker Registry]
    end

    %% User interactions
    USER((User)) --> TG
    TG --> BOT
    BOT --> HANDLERS
    HANDLERS --> FSM

    %% Data flow
    HANDLERS --> SQLITE
    HANDLERS --> REDIS

    %% Task processing
    HANDLERS -->|Queue Task| REDIS
    RUNNER -->|Poll Queue| REDIS
    RUNNER --> PROCESSOR
    PROCESSOR -->|Send Messages| WAPPI
    WAPPI -->|Deliver| WA((WhatsApp Groups))

    %% Payment flow
    HANDLERS -->|Generate Link| ROBO
    ROBO -->|Webhook| BOT
    BOT -->|Update Subscription| SQLITE

    %% Health monitoring
    RUNNER --> HEALTH
    HEALTH -->|Update Status| REDIS

    %% CI/CD flow
    GHA -->|Build & Push| DOCKER
    GHA -->|Deploy| BOT
    GHA -->|Deploy| RUNNER

    classDef service fill:#e1f5fe,stroke:#01579b,stroke-width:2px
    classDef storage fill:#fff3e0,stroke:#e65100,stroke-width:2px
    classDef external fill:#f3e5f5,stroke:#4a148c,stroke-width:2px
    classDef user fill:#e8f5e9,stroke:#1b5e20,stroke-width:2px

    class BOT,RUNNER,HANDLERS,FSM,PROCESSOR,HEALTH service
    class REDIS,SQLITE storage
    class TG,WAPPI,ROBO,GHA,DOCKER external
    class USER,WA user

sequenceDiagram
    participant U as User
    participant B as Bot
    participant R as Redis
    participant TR as Task Runner
    participant W as Wappi API
    participant WG as WhatsApp Groups

    U->>B: /start
    B->>B: Check registration
    B->>U: Main menu

    U->>B: 🚀 Start mailing
    B->>B: Validate subscription
    B->>B: Prepare task data
    B->>R: Queue task<br/>(mailing_tasks_queue)
    B->>R: Save task data<br/>(mailing_task_data:*)
    B->>U: ✅ Mailing started

    loop Task Processing
        TR->>R: BLPOP queue
        R-->>TR: Task ID
        TR->>R: Get task data
        TR->>R: Set lock<br/>(mailing_lock_user_id)
        
        loop For each group
            TR->>R: Check cancellation flag
            alt Not cancelled
                TR->>W: Send text/image
                W->>WG: Deliver message
                W-->>TR: Success/Failure
            else Cancelled
                TR->>R: Clean up
                TR-->>U: ⚠️ Mailing cancelled
            end
        end
        
        TR->>R: Release lock
        TR->>R: Delete task data
        TR->>B: Update keyboard
        TR-->>U: ✅ Mailing completed
    end

flowchart TB
    subgraph "Zero-Downtime Deployment"
        START([New Release]) --> BUILD[Build Image<br/>work_auto_SHA]
        BUILD --> CHECK{Active<br/>Tasks?}
        
        CHECK -->|No| DEPLOY[Deploy All Services]
        CHECK -->|Yes| SPLIT[Deploy Bot Only]
        
        SPLIT --> BOT_NEW[New Bot<br/>Running]
        SPLIT --> TR_OLD[Old Task Runner<br/>Continues]
        
        TR_OLD --> MONITOR[Monitor Tasks]
        MONITOR --> WAIT{Tasks<br/>Complete?}
        WAIT -->|No| MONITOR
        WAIT -->|Yes| UPDATE[Update Task Runner]
        
        UPDATE --> TR_NEW[New Task Runner<br/>Running]
        
        DEPLOY --> DONE([Deployment Complete])
        BOT_NEW --> DONE
        TR_NEW --> DONE
        
        subgraph "Old Stack"
            OLD_BOT[Old Bot<br/>Stopped]
            OLD_TR[Old Task Runner<br/>Graceful Shutdown]
        end
        
        subgraph "New Stack"
            NEW_BOT[New Bot<br/>Active]
            NEW_TR[New Task Runner<br/>Active]
        end
    end

stateDiagram-v2
    [*] --> Idle
    
    Idle --> Registration: /start (new user)
    Idle --> MainMenu: /start (registered)
    
    Registration --> EnterToken: Enter credentials
    EnterToken --> EnterProfileID
    EnterProfileID --> MainMenu: Save user
    
    MainMenu --> MailingData: 📬 Mailing Data
    MainMenu --> StartMailing: 🚀 Start Mailing
    MainMenu --> Subscription: 💎 Subscription
    
    MailingData --> EditText: 📝 Change Text
    MailingData --> EditImages: 🖼️ Change Images
    MailingData --> EditGroups: 👥 Change Groups
    
    EditText --> InputText: Enter text
    InputText --> MailingData: Save
    
    EditImages --> UploadImages: Upload photos
    UploadImages --> CreateCollage: 🌅 Create collage
    CreateCollage --> MailingData: Save
    
    EditGroups --> InputGroups: Enter groups
    InputGroups --> MailingData: Save
    
    StartMailing --> ConfirmMailing: Show preview
    ConfirmMailing --> TaskQueued: ✅ Yes
    ConfirmMailing --> MainMenu: ❌ No
    
    TaskQueued --> Processing: Task Runner
    Processing --> Completed: Success
    Processing --> Cancelled: User cancelled
    Processing --> Failed: Error
    
    Completed --> MainMenu
    Cancelled --> MainMenu
    Failed --> MainMenu
    
    Subscription --> Payment: 💳 Pay
    Payment --> CheckPayment: Robokassa
    CheckPayment --> Subscription: Update status

graph LR
    subgraph "Resource Limits & Throttling"
        GLOBAL[Global Rate Limit<br/>1 msg/sec]
        LOCAL[Local Burst Limit<br/>12 msg/2sec per chat]
        BACKOFF[429 Backoff<br/>Retry-After]
    end
    
    subgraph "Data Storage"
        subgraph "Redis Keys"
            QUEUE[mailing_tasks_queue]
            TASK[mailing_task_data:*]
            LOCK[mailing_lock_*]
            CANCEL[cancel_mailing_*]
            HEALTH[task_runner_health]
            DUP[dup:profile:run:group:sha]
            RATE[wappi_rate:*]
            BACKOFF_KEY[wappi_backoff:*]
        end
        
        subgraph "SQLite Tables"
            USERS[users<br/>- user_id<br/>- instance<br/>- token<br/>- subscription]
            GROUPS[groups<br/>- user_id<br/>- group_name]
            TEXTS[texts<br/>- user_id<br/>- text_message]
            IMAGES[images<br/>- user_id<br/>- image_data]
        end
    end