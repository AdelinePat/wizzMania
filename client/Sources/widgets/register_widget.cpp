#include "widgets/register_widget.hpp"

#include <QDebug>
#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include "utils.hpp"

RegisterWidget::RegisterWidget(QWidget* parent) : QWidget(parent) {
  setObjectName("registerBackground");

  // ── Root: fills the whole widget ──────────────────────────────
  QVBoxLayout* rootLayout = new QVBoxLayout(this);
  rootLayout->setContentsMargins(0, 0, 0, 0);
  rootLayout->setSpacing(0);

  // Scroll area so the card never gets clipped on small windows
  QScrollArea* scroll = new QScrollArea(this);
  scroll->setObjectName("registerScrollArea");
  scroll->setWidgetResizable(true);
  scroll->setFrameShape(QFrame::NoFrame);
  scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

  QWidget* scrollContent = new QWidget();
  scrollContent->setObjectName("registerScrollContent");
  QVBoxLayout* scrollLayout = new QVBoxLayout(scrollContent);
  scrollLayout->setContentsMargins(24, 48, 24, 48);
  scrollLayout->setSpacing(0);

  // ── Card ──────────────────────────────────────────────────────
  QFrame* card = new QFrame(scrollContent);
  card->setObjectName("registerCard");
  card->setFixedWidth(460);

  QVBoxLayout* cardLayout = new QVBoxLayout(card);
  cardLayout->setContentsMargins(40, 40, 40, 40);
  cardLayout->setSpacing(0);

  // ── Logo ──────────────────────────────────────────────────────
  QLabel* logoLabel = new QLabel("WizzMania", card);
  logoLabel->setObjectName("registerLogoLabel");
  logoLabel->setAlignment(Qt::AlignCenter);

  // ── Title ─────────────────────────────────────────────────────
  QLabel* titleLabel = new QLabel("Create your account", card);
  titleLabel->setObjectName("registerTitleLabel");
  titleLabel->setAlignment(Qt::AlignCenter);

  // ── Helper: builds a labeled field block ──────────────────────
  auto makeField = [card](const QString& labelText, QLineEdit* input) -> QWidget* {
    QWidget* block = new QWidget(card);
    block->setObjectName("transparentWidget");
    QVBoxLayout* bl = new QVBoxLayout(block);
    bl->setContentsMargins(0, 0, 0, 0);
    bl->setSpacing(5);
    QLabel* lbl = new QLabel(labelText, block);
    lbl->setObjectName("registerFieldLabel");
    bl->addWidget(lbl);
    bl->addWidget(input);
    return block;
  };

  // ── Fields ────────────────────────────────────────────────────
  usernameInput = new QLineEdit(card);
  usernameInput->setObjectName("registerInput");
  usernameInput->setMinimumHeight(42);
  usernameInput->setPlaceholderText("letters, numbers, underscores");

  emailInput = new QLineEdit(card);
  emailInput->setObjectName("registerInput");
  emailInput->setMinimumHeight(42);
  emailInput->setPlaceholderText("your@email.com");

  passwordInput = new QLineEdit(card);
  passwordInput->setObjectName("registerInput");
  passwordInput->setMinimumHeight(42);
  passwordInput->setEchoMode(QLineEdit::Password);
  passwordInput->setPlaceholderText("min 8 chars");

  confirmPasswordInput = new QLineEdit(card);
  confirmPasswordInput->setObjectName("registerInput");
  confirmPasswordInput->setMinimumHeight(42);
  confirmPasswordInput->setEchoMode(QLineEdit::Password);
  confirmPasswordInput->setPlaceholderText("repeat password");

  // Password fields side by side
  QWidget* passwordRow = new QWidget(card);
  passwordRow->setObjectName("transparentWidget");
  QHBoxLayout* passwordRowLayout = new QHBoxLayout(passwordRow);
  passwordRowLayout->setContentsMargins(0, 0, 0, 0);
  passwordRowLayout->setSpacing(12);
  passwordRowLayout->addWidget(makeField("Password", passwordInput));
  passwordRowLayout->addWidget(makeField("Confirm password", confirmPasswordInput));

  // ── Error label ───────────────────────────────────────────────
  errorLabel = new QLabel(card);
  errorLabel->setObjectName("registerErrorLabel");
  errorLabel->setWordWrap(true);
  errorLabel->setAlignment(Qt::AlignCenter);
  errorLabel->setVisible(false);
  errorLabel->setMinimumHeight(0);

  // ── Submit button ─────────────────────────────────────────────
  createBtn = new QPushButton("Create Account", card);
  createBtn->setObjectName("registerCreateBtn");
  createBtn->setMinimumHeight(44);
  createBtn->setCursor(Qt::PointingHandCursor);
  connect(createBtn, &QPushButton::clicked, this, &RegisterWidget::onCreateClicked);

  // ── Divider + sign-in link ────────────────────────────────────
  QFrame* divider = new QFrame(card);
  divider->setObjectName("registerDivider");
  divider->setFrameShape(QFrame::HLine);
  divider->setFrameShadow(QFrame::Sunken);

  QWidget* signinRow = new QWidget(card);
  signinRow->setObjectName("transparentWidget");
  QHBoxLayout* signinLayout = new QHBoxLayout(signinRow);
  signinLayout->setContentsMargins(0, 0, 0, 0);
  signinLayout->setSpacing(4);

  QLabel* alreadyLabel = new QLabel("Already have an account?", signinRow);
  alreadyLabel->setObjectName("registerSubtitleLabel");

  cancelBtn = new QPushButton("Sign in", signinRow);
  cancelBtn->setObjectName("registerLinkBtn");
  cancelBtn->setFlat(true);
  cancelBtn->setCursor(Qt::PointingHandCursor);
  connect(cancelBtn, &QPushButton::clicked, this, &RegisterWidget::onCancelClicked);

  signinLayout->addStretch();
  signinLayout->addWidget(alreadyLabel);
  signinLayout->addWidget(cancelBtn);
  signinLayout->addStretch();

  // ── Enter-key chain ───────────────────────────────────────────
  connect(usernameInput,        &QLineEdit::returnPressed, this, [this]{ emailInput->setFocus(); });
  connect(emailInput,           &QLineEdit::returnPressed, this, [this]{ passwordInput->setFocus(); });
  connect(passwordInput,        &QLineEdit::returnPressed, this, [this]{ confirmPasswordInput->setFocus(); });
  connect(confirmPasswordInput, &QLineEdit::returnPressed, this, &RegisterWidget::onCreateClicked);

  // ── Assemble card ─────────────────────────────────────────────
  cardLayout->addWidget(logoLabel);
  cardLayout->addSpacing(4);
  cardLayout->addWidget(titleLabel);
  cardLayout->addSpacing(28);
  cardLayout->addWidget(makeField("Username", usernameInput));
  cardLayout->addSpacing(14);
  cardLayout->addWidget(makeField("Email", emailInput));
  cardLayout->addSpacing(14);
  cardLayout->addWidget(passwordRow);
  cardLayout->addSpacing(10);
  cardLayout->addWidget(errorLabel);
  cardLayout->addSpacing(4);
  cardLayout->addWidget(createBtn);
  cardLayout->addSpacing(22);
  cardLayout->addWidget(divider);
  cardLayout->addSpacing(18);
  cardLayout->addWidget(signinRow);

  // ── Center card in scroll area ────────────────────────────────
  scrollLayout->addStretch(1);
  scrollLayout->addWidget(card, 0, Qt::AlignHCenter);
  scrollLayout->addStretch(1);

  scroll->setWidget(scrollContent);
  rootLayout->addWidget(scroll);
}

RegisterWidget::~RegisterWidget() {}

void RegisterWidget::showErrorMessage(const QString& text) { setErrorText(text); }

void RegisterWidget::onCreateClicked() {
  QString username = usernameInput->text().trimmed();
  QString email    = emailInput->text().trimmed();
  QString password = passwordInput->text();
  QString confirm  = confirmPasswordInput->text();
  setErrorText(QString());

  if (username.isEmpty()) { setErrorText("Please enter a username."); usernameInput->setFocus(); return; }
  try {
    std::string cleaned = Utils::clean_username(username.toStdString());
    qInfo() << "[REGISTER] Cleaned username:" << QString::fromStdString(cleaned);
  } catch (const BadInput& e) {
    setErrorText(QString::fromStdString(e.what())); usernameInput->setFocus(); return;
  }
  if (email.isEmpty()) { setErrorText("Please enter an email."); emailInput->setFocus(); return; }
  if (!Utils::is_valid_email(email.toStdString())) {
    setErrorText("Please enter a valid email address."); emailInput->setFocus(); return;
  }
  if (password.isEmpty()) { setErrorText("Please enter a password."); passwordInput->setFocus(); return; }
  if (confirm.isEmpty()) { setErrorText("Please confirm your password."); confirmPasswordInput->setFocus(); return; }
  if (password != confirm) {
    setErrorText("Passwords do not match."); passwordInput->setFocus(); passwordInput->selectAll(); return;
  }
  if (!Utils::is_valid_password(password.toStdString())) {
    setErrorText("Password must be at least 8 characters and contain uppercase, lowercase, a digit, and a special character (!@#$%^&*-_=+;:,.<>?/).");
    passwordInput->setFocus(); passwordInput->selectAll(); return;
  }

  qInfo() << "[REGISTER] Valid: username=" << username << " email=" << email;
  emit registerRequested(username, email, password);
  clearFields();
}

void RegisterWidget::onCancelClicked() {
  clearFields();
  setErrorText(QString());
  emit cancelRequested();
}

void RegisterWidget::setErrorText(const QString& text) {
  errorLabel->setWordWrap(true);
  errorLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
  errorLabel->setText(text);
  if (text.isEmpty()) {
    errorLabel->setMinimumHeight(0);
  } else {
    const int w = errorLabel->width() > 0 ? errorLabel->width() : 380;
    const QRect bounds = errorLabel->fontMetrics().boundingRect(
        QRect(0, 0, w, 10000), Qt::TextWordWrap | Qt::AlignCenter, text);
    errorLabel->setMinimumHeight(bounds.height() + 6);
  }
  errorLabel->setVisible(!text.isEmpty());
}

void RegisterWidget::clearFields() {
  usernameInput->clear();
  emailInput->clear();
  passwordInput->clear();
  confirmPasswordInput->clear();
  errorLabel->clear();
  errorLabel->setVisible(false);
}
