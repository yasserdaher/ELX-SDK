# ELX SDK (العربية)

## هوية المشروع
- **الاسم:** ELX — لغة برمجة ومجموعة أدوات تطوير (SDK)
- **الناشر:** ELECTRON VISION (هوية مشروع؛ لا يُدّعى أنها كيان قانوني مسجّل)
- **الإصدار:** 0.4.1-phase4 (`ELX_SDK_VERSION` في `include/elx_config.h`)
- **الحالة:** تجريبي / معاينة مبكرة للمطورين
- **المستودع:** https://github.com/yasserdaher/ELX-SDK
- **الترخيص:** اتفاقية VOLT 1.26 — راجع [LICENSE](LICENSE) و[LICENSE-ELX](LICENSE-ELX)

## كتالوج الميزات (مبني على الأدلة)
| المكوّن | الحالة | ملاحظات |
|---|---|---|
| واجهة CLI (`elx`) | منفّذ (IMPLEMENTED) | أوامر: build/run/test/init/doctor/version/help/module/package/diag/fmt/lint/debug/freeze/compat/rc/stabilize/release (راجع `cli/elx.c`) |
| سلسلة المترجم | منفّذ جزئياً | lexer/parser/semantic/optimizer/elbc موجودة؛ التحقق عبر اختبارات/أمثلة على Linux |
| وقت التشغيل/VM | منفّذ جزئياً | vm/stack/arena/elbc_reader/runtime/stdlib؛ يشغّل `.elbc` |
| بايت كود ELBC | قيد الاستخدام | `.elbc` ناتج البناء؛ المواصفة في `ELX_SPEC.md` |
| المكتبة القياسية | منفّذ جزئياً | وحدات io/fs/process/env/str/list/map/set/math/error/net/time؛ ليست كل الواجهات موثقة/متحققاً منها |
| الواجهات الأصلية | تجريبي | مجلد `native/` |
| مكوّنات الأمان | تجريبي | مجلد `security/`؛ لا ادعاءات شهادات أمان |
| التشخيص/التنسيق/lint/التصحيح | منفّذ جزئياً | وحدات وأوامر موجودة؛ الاكتمال يعتمد على التنفيذ |
| module/package/spec/compat | تجريبي | أوامر ووحدات أساسية موجودة |
| الأمثلة والاختبارات | منفّذ (Linux) | `examples/` و`make test` |

## البدء السريع (Linux x64 — مُتحقق)
```bash
make clean && make all
./build/elx doctor
./build/elx version
./build/elx init hello_app
./build/elx build hello_app/main.elx
./build/elx run hello_app/main.elx
```
المتوقع: طباعة "Hello from ELX!" وخروج 0 (حسب قالب init).

## دعم الأنظمة (مبني على الأدلة)
| النظام | الحالة | الدليل |
|---|---|---|
| Linux x64 | مُتحقق | بناء محلي + `make test` نجح |
| Windows 10/11 x64 | متوافق للبناء (مصدر) | كود/Makefile/منصات موجودة؛ لا ملف تنفيذي مُتحقق هنا |
| macOS | متوافق للبناء (مصدر) | كود/Makefile/منصات موجودة؛ لا ملف تنفيذي مُتحقق هنا |
| توزيعات Linux | متوافق للبناء (عائلة) | غالباً قابلة للبناء؛ غير مُتحقق لكل توزيعة |
| ARM64 | غير مُتحقق | تعريفات معمارية موجودة؛ لا بناء/اختبار مُتحقق |

## القيود
- لا يُدّعى وجود ثنائيات جاهزة إلا عند التحقق وتغليفها لكل نظام.
- بعض أوامر CLI قد تكون غير مكتملة؛ تحقّق قبل الاعتماد عليها.
- لا ادعاءات أداء/أمان/توافق وهمية.

## روابط
- [التثبيت](docs/ar/INSTALLATION.md) · [CLI](docs/CLI_REFERENCE.md) · [اللغة](docs/ar/language/overview.md) · [المكتبة القياسية](docs/STANDARD_LIBRARY.md) · [المعمارية](docs/ARCHITECTURE.md) · [التنزيلات](docs/DOWNLOADS.md)
